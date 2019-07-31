#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.001f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Minimum;
    glm::vec3 Maximum;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float ScrollSpeed;
    float MouseSensitivity;


    // Constructor with vectors
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f)) :
          Position(position),
          Minimum(-180.0f, -90.0f, 0.11f),
          Maximum(180.0f, 90.0f, 20.0f),
          Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
          Yaw(YAW),
          Pitch(PITCH),
          MovementSpeed(SPEED),
          ScrollSpeed(SPEED),
          MouseSensitivity(SENSITIVITY)
    {
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime * Position.z / 5.0f;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position += Right * velocity;
        if (direction == RIGHT)
            Position -= Right * velocity;
        if (direction == UP)
            Position -= Up * velocity;
        if (direction == DOWN)
            Position += Up * velocity;

        Limit(Position.x, Minimum.x, Maximum.x);
        Limit(Position.y, Minimum.y, Maximum.y);
        Limit(Position.z, Minimum.z, Maximum.z);
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Position -= Right * xoffset * Position.z;
        Position -= Up * yoffset * Position.z;

        Limit(Position.x, Minimum.x, Maximum.x);
        Limit(Position.y, Minimum.y, Maximum.y);
        
        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        float velocity = ScrollSpeed * yoffset * Position.z / 100.0f;
        Position += Front * velocity;
        Limit(Position.z, Minimum.z, Maximum.z);
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }

    void Limit(float& value, float& minimum, float& maximum) {
        if (value < minimum) {
            value = minimum;
        }
        if (value > maximum) {
            value = maximum;
        }
    }
};
#endif
