#pragma once
#include <mscene.hpp>
#include <glm/glm.hpp>

namespace mtx
{
    struct ViewportSettings
    {
	bool enableBlending;
	bool enableDepthTest;
	bool enableCullFace;
	bool enableFillMode;

	ViewportSettings();
    };
    
    class Viewport
    {
        glm::vec4 m_viewport;
        float m_farDistance;
        float m_nearDistance;
        SceneNode* m_cameraNode;

        glm::vec4 m_clearColor;

        glm::mat4 m_perspectiveMatrix;
	glm::mat4 m_inversePerspective;
	
        glm::mat4 m_viewMatrix;
	glm::mat4 m_inverseView;
	
        glm::mat4 m_uiTransform;
        glm::vec3 m_viewPosition;

	ViewportSettings m_settings;
    public:
        Viewport(int resx, int resy);

        void setFarDistance(float dist) { m_farDistance = dist; updatePerspective(); }
        void setNearDistance(float dist) { m_nearDistance = dist; updatePerspective(); }
        void beginViewportFrame();
        void setCameraNode(SceneNode* cameraNode) { m_cameraNode = cameraNode; };
        SceneNode* getCameraNode() { return m_cameraNode; }
        glm::vec4& getViewport() { return m_viewport; }

	ViewportSettings& getSettings() { return m_settings; }
	
        void updateView();
        void updatePerspective();

        void setClearColor(glm::vec4 clear) { m_clearColor = clear; };
        void setPerspective(glm::mat4 m) { m_perspectiveMatrix = m; }	
        glm::mat4& getPerspective() { return m_perspectiveMatrix; }
	glm::mat4 getPerspectiveInverse()
	{
	    return m_inversePerspective;
	}
        void setView(glm::mat4 m) { m_viewMatrix = m; }
        void setViewport(glm::vec4 viewport) { m_viewport = viewport; }
        glm::mat4& getView() { return m_viewMatrix; }
	glm::mat4 getViewInverse() { return m_inverseView; }
    };
}
