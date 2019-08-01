
#include "render.h"

static const float X_SCALE = 0.7f;
static const float DOT_SIZE = 1 / 200.f;

RenderParam::RenderParam(float size, float minSize, float maxSize) :
        m_DotSize(size),
        m_MinSize(minSize),
        m_MaxSize(maxSize),
        m_DotScale(1.0f),
        m_XScale(X_SCALE){

}

void RenderParam::UpdateDotScale(float z) {
    m_DotScale = m_DotSize * z;
}

void RenderParam::UpdateDotSize(float ratio) {
    m_DotSize *= ratio;
    if (m_DotSize < m_MinSize) {
        m_DotSize = m_MinSize;
    }
    if (m_DotSize > m_MaxSize) {
        m_DotSize = m_MaxSize;
    }
}

float RenderParam::GetDotSize() const {
    return m_DotSize;
}

float RenderParam::GetDotScale() const {
    return m_DotScale;
}

float RenderParam::GetXScale() const {
    return m_XScale;
}
