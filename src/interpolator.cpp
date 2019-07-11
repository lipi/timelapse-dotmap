
#include <spdlog/spdlog.h>

#include "interpolator.h"

static const size_t FRAME_SIZE = 70000; // TODO: dynamic size based on DB

Interpolator::Interpolator(const char* filename, size_t frames) :
    m_FrameSize(FRAME_SIZE),
    m_Frames(),
    m_FrameIndex(0) {

    m_FrameProvider.reset(new FrameProvider("frames.db"));
    m_SnapshotBuffer = new glm::vec2[m_FrameSize]; // TODO: smart ptr
    spdlog::debug("snapshot buffer at {}", (void*)m_SnapshotBuffer);
    m_Timestamps = m_FrameProvider->GetTimestamps();
    uint32_t frameIndex = 0;
    spdlog::info("Loading snapshot {} at {}", frameIndex, m_Timestamps[frameIndex]);
    size_t size = m_FrameProvider->GetSnapshot(m_Timestamps[0], m_SnapshotBuffer, m_FrameSize);
    spdlog::info("Loaded {} locations from snapshot {}", size, m_Timestamps[0] );

    for (size_t i = 0; i < frames; i++) {
        auto* pFrame = new glm::vec2[m_FrameSize]; // TODO: smart ptr
        m_Frames.push_back(pFrame);
    }
}

Interpolator::~Interpolator() {
    for (auto pFrame : m_Frames) {
        if (pFrame) { delete pFrame; }
    }
    m_Frames.clear();
    delete m_SnapshotBuffer;
}

glm::vec2* Interpolator::GetSnapshot() {
    return m_SnapshotBuffer;
}

size_t Interpolator::GetSnapshotSize() {
    return m_FrameSize;
}

size_t Interpolator::GetSnapshotSizeBytes() {
    return m_FrameSize * sizeof(glm::vec2);
}

void Interpolator::FillNextFrame(glm::vec2* pFrame, size_t step) {

    for (int i = 0; i < step; i++) {
        unsigned int timestamp = m_Timestamps[m_FrameIndex];
        spdlog::debug("Loading frame {} at {} {}", m_FrameIndex, timestamp, (void*)pFrame);
        size_t size = m_FrameProvider->FillDelta(timestamp, pFrame, m_FrameSize);
        m_FrameIndex++;
        m_FrameIndex %= m_Timestamps.size();
        spdlog::debug("Loaded {} locations from frame {}", size, timestamp);
    }
}