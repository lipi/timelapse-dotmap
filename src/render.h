//
// Created by Andras Lipoth on 2019-07-07.
//

#ifndef TIMELAPSEDOTMAP_RENDER_H
#define TIMELAPSEDOTMAP_RENDER_H

class RenderParam{
public:
    RenderParam();

    float GetDotScale() const;
    float GetXScale() const;

    void UpdateDotScale(float z);

private:
    float m_DotScale;
    float m_XScale;
};

#endif //TIMELAPSEDOTMAP_RENDER_H
