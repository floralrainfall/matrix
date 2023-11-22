#pragma once
#include <mscene.hpp>
#include <mhwabs.hpp>
#include <mmaterial.hpp>
#include <SDL2/SDL_ttf.h>

namespace mtx
{
    struct GUIVertex
    {
        glm::vec2 position;
        glm::vec2 uv;
	glm::vec3 color;
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
	bool m_visible;
	
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
	void setVisible(bool visible) { m_visible = visible; }
        void updateMesh();
        virtual void renderComponent();
        virtual void tick();
        virtual std::string className() { return "GUIItemComponent"; }
    };

    class GUIImageComponent : public GUIItemComponent
    {
    protected:
        HWTextureReference* m_imageTexture;
	bool m_absoluteSize;
        virtual void createGuiMesh();
        virtual void pushRenderProps();
        virtual void popRenderProps();
    public:
        GUIImageComponent(const char* imageName);
        GUIImageComponent(HWTextureReference* ref);

	void setImageTexture(HWTextureReference* ref)
	{
	    m_imageTexture = ref;
	}
	
	void setAbsoluteSize(bool enable)
	{
	    m_absoluteSize = enable;
	    createGuiMesh();
	}
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
	glm::vec4 m_outline;
	glm::vec4 m_background;
        std::string m_text;
	bool m_italics;
	bool m_bold;
	bool m_ttf;
	HWTextureReference* m_ttfTexture;
	TTF_Font* m_ttfFont;
	int m_charactersPerLine;
    public:
        virtual void createGuiMesh();
        virtual void pushRenderProps();
        virtual void popRenderProps();
        GUITextComponent();

	void useTtfFont(const char* font);
	
	void setCharactersPerLine(int p)
	{
	    if(p != m_charactersPerLine)
	    {
		p = m_charactersPerLine;
		updateMesh();
	    }
	};
	
        void setCharacterSize(glm::ivec2 size);
        void setFont(HWTextureReference* texture);
        void setText(std::string text);
	void setBold(bool bold) { m_bold = bold; }
	void setOutline(glm::vec4 outline) { m_outline = outline; }
	void setItalics(bool italics) { m_italics = italics; }

	std::string getText() { return m_text; }
    };
}
