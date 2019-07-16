
#include "replay.h"

const float SPEED = 1.0f;

ReplayParam::ReplayParam() :
        m_Speed(SPEED)
{
}

void ReplayParam::ChangeSpeed(float increase) {
    m_Speed *= 1.0f + increase;
}

float ReplayParam::GetSpeed() {
    return m_Speed;
}
