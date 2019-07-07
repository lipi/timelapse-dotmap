//
// Created by Andras Lipoth on 2019-07-07.
//

#include "render.h"

const float X_SCALE = 0.7f;
const float DOT_SIZE = 1 / 200.f;

RenderParam::RenderParam() :
        m_DotSize(DOT_SIZE),
        m_DotScale(1.0f),
        m_XScale(X_SCALE){

}

void RenderParam::UpdateDotScale(float z) {
    m_DotScale = m_DotSize * z;
}

void RenderParam::UpdateDotSize(float ratio) {
    m_DotSize *= ratio;
}

float RenderParam::GetDotScale() const {
    return m_DotScale;
}

float RenderParam::GetXScale() const {
    return m_XScale;
}
