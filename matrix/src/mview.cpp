#include "mview.hpp"
#include <mapp.hpp>

namespace mtx
{
    Viewport::Viewport(int resx, int resy)
    {
        m_viewport = glm::vec4(0.f);
        m_viewport.z = resx;
        m_viewport.w = resy;

        m_clearColor = glm::vec4(1,0,1,1);

        m_cameraNode = 0;
        m_nearDistance = 0.1f;
        m_farDistance = 100.f;
    }

    void Viewport::beginViewportFrame()
    {
        App::getHWAPI()->gfxViewport(m_viewport);
        App::getHWAPI()->gfxClear(m_clearColor);
        App::getHWAPI()->gfxClearDepth(1.f);

        mtx::HWRenderParameter rp;
        rp.name = "viewport";
        rp.data.v4 = m_viewport;
        rp.type = HWT_VECTOR4;
        App::getHWAPI()->pushParam(rp);
        rp.name = "view_position";
        rp.data.v3 = m_viewPosition;
        rp.type = HWT_VECTOR3;
        App::getHWAPI()->pushParam(rp);
        rp.name = "uitransform";
        rp.data.m4 = m_uiTransform;
        rp.type = HWT_MATRIX4;
        App::getHWAPI()->pushParam(rp);
    }

    void Viewport::updateView()
    {
        SceneNode* camera = getCameraNode();
        if(!camera)
            return;
        SceneTransform cameraTransform = camera->getTransform();
        m_viewMatrix = cameraTransform.getWorldMatrix();
        m_viewPosition = cameraTransform.getPosition();
        m_perspectiveMatrix = glm::perspectiveFov(90.f * ((float)M_PI / 180.f), m_viewport.z, m_viewport.w, m_nearDistance, m_farDistance);
        m_uiTransform = glm::ortho(0.f, m_viewport.z, 0.f, m_viewport.w);
    }
}