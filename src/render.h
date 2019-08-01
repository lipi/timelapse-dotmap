

#ifndef TIMELAPSEDOTMAP_RENDER_H
#define TIMELAPSEDOTMAP_RENDER_H

class RenderParam{
public:
    RenderParam(float size = 0.005f, float minSize = 0.001f, float maxSize = 0.025f);

    float GetDotSize() const;
    float GetDotScale() const;
    float GetXScale() const;

    void UpdateDotScale(float z);
    void UpdateDotSize(float ratio);

private:
    float m_DotSize;
    float m_MinSize;
    float m_MaxSize;
    float m_DotScale;
    float m_XScale;
};

#endif //TIMELAPSEDOTMAP_RENDER_H
