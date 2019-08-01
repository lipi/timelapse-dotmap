
#ifndef TIMELAPSEDOTMAP_REPLAY_H
#define TIMELAPSEDOTMAP_REPLAY_H

class ReplayParam {
public:
    ReplayParam(float speed = 1.0, float minSpeed = 0.1f, float maxSpeed = 120.0f);
    float GetSpeed();

    void ChangeSpeed(float increase);
    void SetDotScale(float dotScale);

private:
    float m_Speed;
    float m_MaxSpeed;
    float m_MinSpeed;
};
#endif //TIMELAPSEDOTMAP_REPLAY_H
