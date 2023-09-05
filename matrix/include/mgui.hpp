#pragma once
#include <mscene.hpp>
#include <mhwabs.hpp>
#include <mmaterial.hpp>

namespace mtx
{
    struct GUIVertex
    {
        glm::vec2 position;
        glm::vec2 uv;
    };

    class GUIItemComponent : public SceneComponent
    {
    protected:
        HWLayoutReference* m_layout;
        HWBufferReference* m_vertices;
        Material* m_material;
        glm::vec3 m_offset;
        glm::vec2 m_size;
        int m_vertexCount;
        virtual void createGuiMesh() {};
        virtual void pushRenderProps() {};
        virtual void popRenderProps() {};
    public:
        GUIItemComponent();

        void setMaterial(Material* material) { m_material = material; };
        void setOffset(glm::vec3 offset) { m_offset = offset; };
        void setSize(glm::vec2 size) { m_size = size; }
        void updateMesh();
        virtual void renderComponent();
        virtual std::string className() { return "GUIItemComponent"; }
    };

    class GUIImageComponent : public GUIItemComponent
    {
    protected:
        HWTextureReference* m_imageTexture;
        virtual void createGuiMesh();
        virtual void pushRenderProps();
        virtual void popRenderProps();
    public:
        GUIImageComponent(const char* imageName);
    };
}