
#ifndef TIMELAPSEDOTMAP_REPLAY_H
#define TIMELAPSEDOTMAP_REPLAY_H

class ReplayParam {
public:
    ReplayParam();
    float GetSpeed();

    void ChangeSpeed(float increase);
    void SetDotScale(float dotScale);

private:

    float m_Speed;
};
#endif //TIMELAPSEDOTMAP_REPLAY_H
