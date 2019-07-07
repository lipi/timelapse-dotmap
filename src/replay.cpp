
#include "replay.h"


const float SPEED = 5.0f;

ReplayParam::ReplayParam() :

        m_Speed(SPEED)
        {
}



void ReplayParam::ChangeSpeed(float increase) {
    m_Speed *= 1.0f + increase;
    if (m_Speed < 1.0f) {m_Speed = 1.0f;}
}

float ReplayParam::GetSpeed() {
    return m_Speed;
}
