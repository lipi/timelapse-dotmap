
#include "replay.h"

const float SPEED = 1.0f;

ReplayParam::ReplayParam(float speed, float minSpeed, float maxSpeed) :
        m_Speed(speed),
        m_MinSpeed(minSpeed),
        m_MaxSpeed(maxSpeed)
{
}

void ReplayParam::ChangeSpeed(float increase) {
    m_Speed *= 1.0f + increase;
    if (m_Speed > m_MaxSpeed) {
        m_Speed = m_MaxSpeed;
    }
    if (m_Speed < m_MinSpeed) {
        m_Speed = m_MinSpeed;
    }
}

float ReplayParam::GetSpeed() {
    return m_Speed;
}
