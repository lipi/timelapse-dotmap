//
// Created by Andras Lipoth on 2019-07-07.
//

#include "render.h"

const float X_SCALE = 0.7f;
const float DOT_SCALE = 1 / 200.f;


RenderParam::RenderParam() :
        m_DotScale(DOT_SCALE),
        m_XScale(X_SCALE){

}

void RenderParam::UpdateDotScale(float z) {
    m_DotScale = DOT_SCALE * z;
}

float RenderParam::GetDotScale() const {
    return m_DotScale;
}


float RenderParam::GetXScale() const {
    return m_XScale;
}
