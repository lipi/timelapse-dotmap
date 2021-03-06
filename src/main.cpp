#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/model.h>

#include <iostream>

#include <INIReader.h>

#include "camera.h"
#include "replay.h"
#include "render.h"
#include "frame.h"
#include "queue.h"
#include "interpolator.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void printHUD(time_t epochTime);

INIReader config;
ReplayParam replay;
RenderParam render;
Camera camera;

float screenWidth;
float screenHeight;
float lastX;
float lastY;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

char* iniFilename;
string dbFilename;

INIReader readIni(char *filename) {
    INIReader reader(filename);

    dbFilename = reader.Get("database", "filename", "frames.fb");

    screenWidth = reader.GetReal("window", "width", 960);
    screenHeight = reader.GetReal("window", "height", 540);

    lastX = screenWidth / 2;
    lastY = screenHeight / 2;

    camera.Position = glm::vec3(reader.GetReal("camera", "x", 174.0f) * render.GetXScale(),
                                reader.GetReal("camera", "y", -40.0f),
                                reader.GetReal("camera", "z", 10.0f));

    camera.Minimum = glm::vec3(reader.GetReal("camera", "x_min", -180.0f) * render.GetXScale(),
                               reader.GetReal("camera", "y_min", -90.0f),
                               reader.GetReal("camera", "z_min", 0.11f));

    camera.Maximum = glm::vec3(reader.GetReal("camera", "x_max", 180.0f) * render.GetXScale(),
                               reader.GetReal("camera", "y_max", 90.0f),
                               reader.GetReal("camera", "z_max", 20.0f));

    camera.MovementSpeed = reader.GetReal("mouse", "pan_speed", 2.5);
    camera.ScrollSpeed = reader.GetReal("mouse", "scroll_speed", 2.5);
    if (reader.GetBoolean("mouse", "scroll_inverted", false)) {
        camera.ScrollSpeed *= -1;
    }

    replay = ReplayParam(reader.GetReal("replay", "speed", 1.0f),
                         reader.GetReal("replay", "speed_min", 0.1f),
                         reader.GetReal("replay", "speed_max", 120.0f));

    render = RenderParam(reader.GetReal("render", "dotsize", 0.005f),
                         reader.GetReal("render", "dotsize_min", 0.001f),
                         reader.GetReal("render", "dotsize_max", 0.025f));

    return reader;
}

int main(int argc, char* argv[])
{
    if (argc < 2 ){
        fprintf(stderr, "Usage:   %s <inifile>\n", argv[0]);
        fprintf(stderr, "Example: %s config.ini\n", argv[0]);
        exit(1);
    }
    spdlog::set_level(spdlog::level::info);

    iniFilename = argv[1];
    config = readIni(iniFilename);
    FrameProvider frameProvider(dbFilename.c_str());
    spdlog::info("Creating frame queue...");
    FrameQueue frameQueue(frameProvider, 120);
    Interpolator interpolator(frameQueue);
    spdlog::info("Creating window...");
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "TLDM", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    Shader dotShader("dots.vs", "dots.fs");
    Model dot(FileSystem::getPath("resources/dot/dot.obj")); // FIXME: move resources during build

    // configure instanced array
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, frameQueue.GetFrameSizeBytes(),
            frameQueue.OldestFrame(), GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    for (unsigned int i = 0; i < dot.meshes.size(); i++)
    {
        unsigned int VAO = dot.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for vec2
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

        glVertexAttribDivisor(3, 1);

        glBindVertexArray(0);
    }
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        static float phase = 0.0f;
        phase += replay.GetSpeed();
        while (phase >= 1.0f) {
            phase -= 1.0f;
            frameProvider.Next(frameQueue.LastFrame());
        }
        interpolator.Interpolate();
        void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        frameQueue.Pop((glm::vec2*)ptr);
        glUnmapBuffer(GL_ARRAY_BUFFER);

        float currentFrame;
        while ((currentFrame = glfwGetTime()) < lastFrame + (1 / 60.f)) {/* do nothing */}
        deltaTime = currentFrame - lastFrame;
        printHUD(frameProvider.CurrentTimestamp());
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure transformation matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.0001f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        render.UpdateDotScale(camera.Position.z);

        dotShader.use();
        dotShader.setMat4("projection", projection);
        dotShader.setMat4("view", view);
        dotShader.setFloat("dot_scale", render.GetDotScale());
        dotShader.setFloat("x_scale", render.GetXScale());
        
        // draw dots
        dotShader.use();
        dotShader.setInt("texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dot.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for (unsigned int i = 0; i < dot.meshes.size(); i++)
        {
            glBindVertexArray(dot.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, dot.meshes[i].indices.size(),
                    GL_UNSIGNED_INT, nullptr, frameQueue.GetFrameSize());
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        replay.ChangeSpeed(-deltaTime);
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        replay.ChangeSpeed(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        render.UpdateDotSize(1.0f - deltaTime);
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        render.UpdateDotSize(1.0f + deltaTime);

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        readIni(iniFilename);


}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
    spdlog::info("screen: {} x {}", width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    static float leftDown;
    static float rightDown;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftDown = glfwGetTime();
        }
        if (action == GLFW_RELEASE) {
            replay.ChangeSpeed(leftDown - glfwGetTime()); // negative
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightDown = glfwGetTime();
        }
        if (action == GLFW_RELEASE) {
            replay.ChangeSpeed(glfwGetTime() - rightDown);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // TODO: slider
    }
}

void printHUD(time_t epochTime) {
    char buffer[80];
    strftime (buffer, 80, "%F %T UTC", std::localtime(&epochTime));
    printf("%s, dot: %.4f speed: %.2f x:%.3f y:%.3f z:%.3f FPS:%3.1f \r",
           buffer,
           render.GetDotSize(), replay.GetSpeed(),
           camera.Position.x / render.GetXScale(), camera.Position.y, camera.Position.z,
           1 / deltaTime);
    fflush(stdout);
}