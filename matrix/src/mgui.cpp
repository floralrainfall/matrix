#include <mgui.hpp>
#include <mapp.hpp>
#include <mdev.hpp>
#include <SDL2/SDL_ttf.h>

namespace mtx
{
    GUIItemComponent::GUIItemComponent()
    {
        m_vertices = App::getHWAPI()->newBuffer();        
        m_layout = App::getHWAPI()->newLayout();
        m_vertexCount = 0;
        m_material = 0;
        m_offset = glm::vec3();
        m_size = glm::vec2(1.f);
        m_color = glm::vec4(1.f);
	m_visible = true;
    }

    void GUIItemComponent::renderComponent()
    {
        if(!m_material)
            return;
	if(!m_visible)
	    return;

        pushRenderProps();

        HWRenderParameter rp;
        rp.name = "offset";
        rp.type = HWT_VECTOR3;
        rp.data.v3 = m_absoluteOffset;
        App::getHWAPI()->pushParam(rp);
        rp.name = "scale";
        rp.type = HWT_VECTOR2;
        rp.data.v2 = m_size;
        App::getHWAPI()->pushParam(rp);
        rp.name = "color";
        rp.type = HWT_VECTOR4;
        rp.data.v4 = m_color;
        App::getHWAPI()->pushParam(rp);

        App::getHWAPI()->gfxDrawArrays(HWAPI::HWPT_TRIANGLES, m_layout, m_vertexCount, m_material->getProgram());
        popRenderProps();
    }

    void GUIItemComponent::tick()
    {
        m_node->setOccludes(false);
    }

    void GUIItemComponent::updateMesh()
    {
        createGuiMesh();
        m_layout->clearEntries();
        m_layout->addEntry({m_vertices, 2, HWT_FLOAT, false, sizeof(GUIVertex), (void*)offsetof(GUIVertex, position)});
        m_layout->addEntry({m_vertices, 2, HWT_FLOAT, false, sizeof(GUIVertex), (void*)offsetof(GUIVertex, uv)});
        m_layout->addEntry({m_vertices, 3, HWT_FLOAT, false, sizeof(GUIVertex), (void*)offsetof(GUIVertex, color)});
        m_layout->upload();
    }

    GUIImageComponent::GUIImageComponent(const char* imageName)
    {
        m_imageTexture = App::getHWAPI()->loadCachedTexture(imageName);
        setMaterial(Material::getMaterial("materials/gui/GUIImageComponent.mmf"));
        m_absoluteSize = false;
	
        updateMesh();
    }

    GUIImageComponent::GUIImageComponent(HWTextureReference* ref)
    {
        m_imageTexture = ref;        
        setMaterial(Material::getMaterial("materials/gui/GUIImageComponent.mmf"));
        m_absoluteSize = false;
	
        updateMesh();
    }

    glm::vec3 GUIItemComponent::getAbsoluteOffset()
    {
        return m_offset;

        if(!m_node)
            return m_offset;
        if(!m_node->getParent())
            return m_offset;
        GUIItemComponent* compparent = (GUIItemComponent*)m_node->getParent()->getComponent("GUIItemComponent");
        glm::vec3 v = m_offset;
        if(compparent)
            v += compparent->getAbsoluteOffset();
        return v;
    }

    void GUIImageComponent::createGuiMesh()
    {
        std::vector<GUIVertex> guivertices;
        GUIVertex v;
        glm::vec2 ts = m_imageTexture->getTextureSize();
	if(m_absoluteSize)
	    ts = glm::vec2(1.0,1.0);

        v.position = glm::vec2(0.0,0.0);
        v.uv       = glm::vec2(0.0,0.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);

        v.position = glm::vec2(ts.x,0.0);
        v.uv       = glm::vec2(1.0,0.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,ts.y);
        v.uv       = glm::vec2(0.0,1.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);


        v.position = glm::vec2(ts.x,0.0);
        v.uv       = glm::vec2(1.0,0.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);


        v.position = glm::vec2(ts.x,ts.y);
        v.uv       = glm::vec2(1.0,1.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,ts.y);
        v.uv       = glm::vec2(0.0,1.0);
	v.color    = glm::vec3(1);
        guivertices.push_back(v);

        m_vertices->upload(guivertices.size() * sizeof(GUIVertex), guivertices.data());
        m_vertexCount = guivertices.size();
    }

    void GUIImageComponent::pushRenderProps()
    {
        HWRenderParameter rp;
        rp.name = "texture";
        rp.type = HWT_TEXTURE;
        rp.data.tx.tx = m_imageTexture;
        rp.data.tx.slot = 0;
        App::getHWAPI()->pushParam(rp);
    }

    void GUIImageComponent::popRenderProps()
    {
        App::getHWAPI()->popParam();
    }

    GUITextComponent::GUITextComponent()
    {
        m_texture = 0;
        m_characterSize = glm::ivec2(8.f, 16.f);
	m_outline = glm::vec4(0);
	m_background = glm::vec4(0.0);
        setMaterial(Material::getMaterial("materials/gui/GUITextComponent.mmf"));
	m_charactersPerLine = 0;
	m_ttfTexture = NULL;
	m_ttf = false;
	m_bold = false;
	m_italics = false;

	//useTtfFont("/usr/share/fonts/Windows/comic.ttf");
	
        updateMesh();
    }

    void GUITextComponent::createGuiMesh()
    {
        if(!m_texture)
            return;
   
        std::vector<GUIVertex> guivertices;
        GUIVertex v;

	if(m_ttf)
	{
	    if(m_text.size() == 0)
		return;
	    
	    SDL_Surface* ttfSurfBase =
		TTF_RenderUTF8_Blended_Wrapped(m_ttfFont,
					       m_text.c_str(),
					       {
						   .r = 255,
						   .g = 255,
						   .b = 255,
						   .a = 255
					       },
					       m_charactersPerLine*16);
	    if(!ttfSurfBase)
	    {
		DEV_SOFTWARN("could not render TTF, "
			     "falling back to bitmap "
			     "reason: %s",
			     SDL_GetError());
		m_ttf = false;
		updateMesh();
		return;
	    }

	    SDL_Surface* ttfSurf =
		SDL_ConvertSurfaceFormat(ttfSurfBase,
					 SDL_PIXELFORMAT_ABGR8888,
					 0);
	    
	    
	    glm::vec2 sz = glm::vec2(ttfSurf->w, ttfSurf->h);
	    m_ttfTexture->upload(glm::ivec2(
				     ttfSurf->w,
				     ttfSurf->h),
				 ttfSurf->pixels, false);
	    
	    SDL_FreeSurface(ttfSurf);
	    SDL_FreeSurface(ttfSurfBase);

	    glm::vec2 corigin = glm::vec2(0,-ttfSurf->h+16.0);
	    glm::vec2 uvoffset = glm::vec2(0);
	    glm::vec3 ccolor = glm::vec3(0);

	    v.position = corigin + glm::vec2(0.0,0.0);
	    v.uv       = uvoffset + glm::vec2(0.0,1.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);

	    v.position = corigin + glm::vec2(sz.x,0.0);
	    v.uv       = uvoffset + glm::vec2(1.0,1.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);

	    v.position = corigin + glm::vec2(0.0,sz.y);
	    v.uv       = uvoffset + glm::vec2(0.0,0.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);

	    v.position = corigin + glm::vec2(sz.x,0.0);
	    v.uv       = uvoffset + glm::vec2(1.0,1.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);

	    v.position = corigin + glm::vec2(sz.x,sz.y);
	    v.uv       = uvoffset + glm::vec2(1.0,0.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);

	    v.position = corigin + glm::vec2(0.0,sz.y);
	    v.uv       = uvoffset + glm::vec2(0.0,0.0);
	    v.color    = ccolor;
	    guivertices.push_back(v);	    
	}
	else
	{
	    m_textureSize = m_texture->getTextureSize();
	    m_textureUvSize.x = (float)m_characterSize.x / (float)m_textureSize.x;
	    m_textureUvSize.y = (float)m_characterSize.y / (float)m_textureSize.y;

	    int charactersPer = m_textureSize.x / m_characterSize.x;
	    if(charactersPer == 0)
		return;

	    glm::vec3 ccolor = glm::vec3(1);

	    int line = 0;
	    int col = 0;
	    for(int i = 0; i < m_text.size(); i++)
	    {
		char c = m_text[i];

		if(c == '\x27')
		{
		    unsigned char r = m_text[++i]+1;
		    unsigned char g = m_text[++i]+1;
		    unsigned char b = m_text[++i]+1;
		    if(r == 0xff && g == 0xff && b == 0xff)
		    {
			ccolor = glm::vec3(1);
		    }
		    else
		    {
			ccolor = glm::vec3(
			    r / 255.0,
			    g / 255.0,
			    b / 255.0);
		    }
		    continue;
		}

		if(m_charactersPerLine && col > m_charactersPerLine)
		{
		    line++;
		    col = 0;
		}
	    
		glm::vec2 corigin = glm::vec2(
		    m_characterSize.x * col,
		    -m_characterSize.y * line
		    );

		glm::vec2 uvoffset = glm::vec2(
		    m_textureUvSize.x * (c % (charactersPer)),
		    1.0 - m_textureUvSize.y * ((c / (charactersPer)) + 1)
		    );

		if(c == '\n')
		{
		    line++;
		    col = 0;
		    continue;
		}

		col++;
	    
		v.position = corigin + glm::vec2(0.0,0.0);
		v.uv       = uvoffset + glm::vec2(0.0,0.0);
		v.color    = ccolor;
		guivertices.push_back(v);

		v.position = corigin + glm::vec2(m_characterSize.x,0.0);
		v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,0.0);
		v.color    = ccolor;
		guivertices.push_back(v);

		v.position = corigin + glm::vec2(0.0,m_characterSize.y);
		v.uv       = uvoffset + glm::vec2(0.0,m_textureUvSize.y);
		v.color    = ccolor;
		guivertices.push_back(v);

		v.position = corigin + glm::vec2(m_characterSize.x,0.0);
		v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,0.0);
		v.color    = ccolor;
		guivertices.push_back(v);

		v.position = corigin + glm::vec2(m_characterSize.x,m_characterSize.y);
		v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,m_textureUvSize.y);
		v.color    = ccolor;
		guivertices.push_back(v);

		v.position = corigin + glm::vec2(0.0,m_characterSize.y);
		v.uv       = uvoffset + glm::vec2(0.0,m_textureUvSize.y);
		v.color    = ccolor;
		guivertices.push_back(v);

	    }
	}

        m_vertices->upload(guivertices.size() * sizeof(GUIVertex), guivertices.data());
        m_vertexCount = guivertices.size();
    }

    void GUITextComponent::pushRenderProps()
    {
        HWRenderParameter rp;
        rp.name = "texture";
        rp.type = HWT_TEXTURE;
	if(m_ttf)
	    rp.data.tx.tx = m_ttfTexture;        
	else
	    rp.data.tx.tx = m_texture;        
        rp.data.tx.slot = 0;
        App::getHWAPI()->pushParam(rp);
        rp.name = "outline";
        rp.type = HWT_VECTOR4;
	rp.data.v4 = m_outline;
	App::getHWAPI()->pushParam(rp);
        rp.name = "background";
        rp.type = HWT_VECTOR4;
	rp.data.v4 = m_background;
        App::getHWAPI()->pushParam(rp);
    }

    void GUITextComponent::popRenderProps()
    {
        App::getHWAPI()->popParam();
        App::getHWAPI()->popParam();
        App::getHWAPI()->popParam();
    }

    void GUITextComponent::setFont(HWTextureReference* texture)
    {
        m_texture = texture;
        updateMesh();
    }

    void GUITextComponent::setText(std::string text)
    {
	if(m_text != text)
	{
	    m_text = text;
	    updateMesh();
	}
    }

    void GUITextComponent::setCharacterSize(glm::ivec2 size)
    {
        m_characterSize = size;
        updateMesh();
    }

    void GUITextComponent::useTtfFont(const char* font)
    {
	if(m_ttf)
	    return;
	m_ttf = true;
	m_ttfFont =
	    TTF_OpenFont(App::getFileSystem()->fullPath(font).c_str(),
			 16);
	if(!m_ttfFont)
	    DEV_ERROR("error loading ttf font %s", font);
	m_ttfTexture = App::getHWAPI()->newTexture();
	updateMesh();
    }
}
