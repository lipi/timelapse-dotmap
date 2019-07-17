

#ifndef TIMELAPSEDOTMAP_RENDER_H
#define TIMELAPSEDOTMAP_RENDER_H

class RenderParam{
public:
    RenderParam();

    float GetDotScale() const;
    float GetXScale() const;

    void UpdateDotScale(float z);
    void UpdateDotSize(float ratio);

private:
    float m_DotSize;
    float m_DotScale;
    float m_XScale;
};

#endif //TIMELAPSEDOTMAP_RENDER_H
