#ifndef TIMELAPSEDOTMAP_QUEUE_H
#define TIMELAPSEDOTMAP_QUEUE_H

#include <glm/glm.hpp>

#include "frame.h"

class FrameQueue {

public:
    FrameQueue(FrameProvider& frameProvider, size_t frameSize);
    ~FrameQueue();

    size_t Size();
    size_t GetFrameSize();
    size_t GetFrameSizeBytes();

    glm::vec2* OldestFrame();
    glm::vec2* LastFrame();
    glm::vec2* PreviousFrame();

    void Pop(glm::vec2* frame);

    std::vector<glm::vec2*> m_Frames;
private:
    FrameProvider& m_FrameProvider;
    size_t m_FrameSize;
};

#endif //TIMELAPSEDOTMAP_QUEUE_H
