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
        glm::vec3 m_absoluteOffset;
        glm::vec2 m_size;
        glm::vec4 m_color;
        int m_vertexCount;
        virtual void createGuiMesh() {};
        virtual void pushRenderProps() {};
        virtual void popRenderProps() {};
    public:
        GUIItemComponent();

        glm::vec3 getAbsoluteOffset();

        void setMaterial(Material* material) { m_material = material; };
        void setOffset(glm::vec3 offset) { m_offset = offset; m_absoluteOffset = getAbsoluteOffset(); };
        void setSize(glm::vec2 size) { m_size = size; }
        void setColor(glm::vec4 color) { m_color = color; }
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
        GUIImageComponent(HWTextureReference* ref);
    };

    struct GUIFontCharacter
    {
    };

    struct GUIFont
    {

    public:
        GUIFont();
    };

    class GUITextComponent : public GUIItemComponent
    {
        HWTextureReference* m_texture;
        glm::ivec2 m_textureSize;
        glm::ivec2 m_characterSize;
        glm::vec2 m_textureUvSize;
        std::string m_text;
    public:
        virtual void createGuiMesh();
        virtual void pushRenderProps();
        virtual void popRenderProps();
        GUITextComponent();

        void setCharacterSize(glm::ivec2 size);
        void setFont(HWTextureReference* texture);
        void setText(std::string text);
    };
}