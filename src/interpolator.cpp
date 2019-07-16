
#include <spdlog/spdlog.h>

#include "queue.h"
#include "interpolator.h"


// do not interpolate if jump is more than this
static constexpr const float MAX_DISTANCE_DEGREE = 0.01f;

Interpolator::Interpolator(FrameQueue& queue) :
        m_Q(queue),
        m_FrameSize(queue.GetFrameSize())
{
}

uint Interpolator::FindChangeIndex(uint pointIndex) {
    glm::vec2 point = m_Q.PreviousFrame()[pointIndex];

    // find earliest frame index where the point's value was the same
    for (int i = m_Q.Size() - 2; i > 0; i--) {
        glm::vec2* frame = m_Q.m_Frames[i];
        if (frame[pointIndex].x != point.x ||
            frame[pointIndex].y != point.y){
            return i + 1;
        }
    }
    return 0;
}

void Interpolator::Interpolate() {

    // collect some stats
    uint pointCounter = 0;
    uint stepCounter = 0;
    uint maxRange = 0;

    // check each point: has it changed since previous?
    for (uint i = 0; i < m_FrameSize; i++) {
        if (m_Q.LastFrame()[i].x != m_Q.PreviousFrame()[i].x ||
            m_Q.LastFrame()[i].y != m_Q.PreviousFrame()[i].y) {

            pointCounter++;

            // find the range to interpolate over
            uint changeIndex = FindChangeIndex(i);
            glm::vec2 start = m_Q.m_Frames[changeIndex][i];
            glm::vec2 end = m_Q.LastFrame()[i];

            // calculate delta per step for interpolation
            uint range = m_Q.Size() - 1 - changeIndex;
            if (range > maxRange) {maxRange = range;}
            glm::vec2 delta = end - start;
            if ((delta.x * delta.x + delta.y * delta.y) >
                MAX_DISTANCE_DEGREE * MAX_DISTANCE_DEGREE) {
                // do not interpolate if delta is more than ~1km
                continue;
            }

            // delta.x = (end.x - start.x) / float(range);
            // delta.y = (end.y - start.y) / float(range);
            delta /= float(range);

            // interpolate from change
            for (uint j = changeIndex + 1, n = 1; j < m_Q.Size() - 1; j++, n++) {
                m_Q.m_Frames[j][i] += (delta * float(n));
                stepCounter++;
            }
        }
    }
    spdlog::debug("Interpolated {} points in {} steps, avg range {:.4f} max range {}",
            pointCounter, stepCounter, float(stepCounter)/float(pointCounter), maxRange);
}