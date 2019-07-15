
#ifndef TIMELAPSEDOTMAP_INTERPOLATOR_H
#define TIMELAPSEDOTMAP_INTERPOLATOR_H

#include <cstdlib>

#include <glm/gtc/type_ptr.hpp>

#include "frame.h"
#include "update.h"

class Interpolator {

public:
    Interpolator(FrameQueue& frameQueue);

    void Interpolate();

private:

    uint FindChangeIndex(uint pointIndex);

    FrameQueue& m_Q;
    size_t m_FrameSize;
};
#endif //TIMELAPSEDOTMAP_INTERPOLATOR_H
