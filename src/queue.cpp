
#include <spdlog/spdlog.h>

#include "queue.h"

FrameQueue::FrameQueue(FrameProvider& frameProvider, size_t queueSize) :
        m_FrameProvider(frameProvider),
        m_FrameSize(frameProvider.GetFrameSize()) {

    assert(queueSize > 1);

    for (size_t i = 0; i < queueSize; i++) {
        auto *pFrame = new glm::vec2[m_FrameSize]; // TODO: smart ptr
        m_Frames.push_back(pFrame);
        m_FrameProvider.GetSnapshot(1556625600, pFrame, GetFrameSize());
    }
    spdlog::info("Frame queue has been initialised");
}

FrameQueue::~FrameQueue() {
}

void FrameQueue::Pop(glm::vec2* frame) {
    // save the oldest frame's pointer, will be reused
    glm::vec2* oldestFrame = m_Frames[0];
    // pass the oldest frame back to caller
    memcpy(frame, oldestFrame, GetFrameSizeBytes());

    // move every frame forward one step
    for (uint i = 0; i < m_Frames.size() - 1; i++) {
        m_Frames[i] = m_Frames[i + 1];
    }
    uint last = m_Frames.size() - 1;
    // reuse oldest frame as last
    m_Frames[last] = oldestFrame;

    // populate last frame
    memcpy(m_Frames[last], m_Frames[last-1], GetFrameSizeBytes());
}

size_t FrameQueue::Size() {
    return m_Frames.size();
}

size_t FrameQueue::GetFrameSize() {
    return m_FrameSize;
}

size_t FrameQueue::GetFrameSizeBytes() {
    return m_FrameSize * sizeof(glm::vec2);
}

glm::vec2 *FrameQueue::OldestFrame() {
    return m_Frames.front();
}

glm::vec2 *FrameQueue::LastFrame() {
    return m_Frames.back();
}

glm::vec2 *FrameQueue::PreviousFrame() {
    uint last = m_Frames.size() - 1;
    return m_Frames[last - 1];
}

