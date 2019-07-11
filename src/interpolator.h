
#ifndef TIMELAPSEDOTMAP_INTERPOLATOR_H
#define TIMELAPSEDOTMAP_INTERPOLATOR_H

#include <cstdlib>

#include <glm/gtc/type_ptr.hpp>

#include "frame.h"
#include "update.h"

class Interpolator {

public:
    Interpolator(const char* filename, size_t frames);

    glm::vec2* GetSnapshot();
    size_t GetSnapshotSize();
    size_t GetSnapshotSizeBytes();
    void FillNextFrame(glm::vec2* pFrame, size_t size);

    virtual ~Interpolator();

private:
    std::unique_ptr<FrameProvider> m_FrameProvider;
    glm::vec2* m_SnapshotBuffer;
    size_t m_FrameSize;
    std::vector<glm::vec2*> m_Frames;
    std::vector<uint32_t> m_Timestamps;
    unsigned int m_FrameIndex;
};
#endif //TIMELAPSEDOTMAP_INTERPOLATOR_H
