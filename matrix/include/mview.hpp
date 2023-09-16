#pragma once
#include <mscene.hpp>
#include <glm/glm.hpp>

namespace mtx
{
    class Viewport
    {
        glm::vec4 m_viewport;
        float m_farDistance;
        float m_nearDistance;
        SceneNode* m_cameraNode;

        glm::vec4 m_clearColor;

        glm::mat4 m_perspectiveMatrix;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_uiTransform;
        glm::vec3 m_viewPosition;
    public:
        Viewport(int resx, int resy);

        void setFarDistance(float dist) { m_farDistance = dist; updateView(); }
        void setNearDistance(float dist) { m_nearDistance = dist; updateView(); }
        void beginViewportFrame();
        void setCameraNode(SceneNode* cameraNode) { m_cameraNode = cameraNode; };
        SceneNode* getCameraNode() { return m_cameraNode; }
        glm::vec4& getViewport() { return m_viewport; }

        void updateView();

        void setClearColor(glm::vec4 clear) { m_clearColor = clear; };
        void setPerspective(glm::mat4 m) { m_perspectiveMatrix = m; }
        glm::mat4& getPerspective() { return m_perspectiveMatrix; }
        void setView(glm::mat4 m) { m_viewMatrix = m; }
        glm::mat4& getView() { return m_viewMatrix; }
    };
}