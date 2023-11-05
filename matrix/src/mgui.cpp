#include <mgui.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

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
    }

    void GUIItemComponent::renderComponent()
    {
        if(!m_material)
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
        m_layout->upload();
    }

    GUIImageComponent::GUIImageComponent(const char* imageName)
    {
        m_imageTexture = App::getHWAPI()->loadCachedTexture(imageName);
        setMaterial(Material::getMaterial("materials/gui/GUIImageComponent.mmf"));
        
        updateMesh();
    }

    GUIImageComponent::GUIImageComponent(HWTextureReference* ref)
    {
        m_imageTexture = ref;        
        setMaterial(Material::getMaterial("materials/gui/GUIImageComponent.mmf"));
        
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

        v.position = glm::vec2(0.0,0.0);
        v.uv       = glm::vec2(0.0,0.0);
        guivertices.push_back(v);

        v.position = glm::vec2(ts.x,0.0);
        v.uv       = glm::vec2(1.0,0.0);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,ts.y);
        v.uv       = glm::vec2(0.0,1.0);
        guivertices.push_back(v);


        v.position = glm::vec2(ts.x,0.0);
        v.uv       = glm::vec2(1.0,0.0);
        guivertices.push_back(v);


        v.position = glm::vec2(ts.x,ts.y);
        v.uv       = glm::vec2(1.0,1.0);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,ts.y);
        v.uv       = glm::vec2(0.0,1.0);
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
        setMaterial(Material::getMaterial("materials/gui/GUITextComponent.mmf"));

        updateMesh();
    }

    void GUITextComponent::createGuiMesh()
    {
        if(!m_texture)
            return;
   
        std::vector<GUIVertex> guivertices;
        GUIVertex v;

        m_textureSize = m_texture->getTextureSize();
        m_textureUvSize.x = (float)m_characterSize.x / (float)m_textureSize.x;
        m_textureUvSize.y = (float)m_characterSize.y / (float)m_textureSize.y;

        int charactersPer = m_textureSize.x / m_characterSize.x;
	if(charactersPer == 0)
	    return;

        int line = 0;
        int col = 0;
        for(int i = 0; i < m_text.size(); i++)
        {
            char c = m_text[i];

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
            guivertices.push_back(v);

            v.position = corigin + glm::vec2(m_characterSize.x,0.0);
            v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,0.0);
            guivertices.push_back(v);

            v.position = corigin + glm::vec2(0.0,m_characterSize.y);
            v.uv       = uvoffset + glm::vec2(0.0,m_textureUvSize.y);
            guivertices.push_back(v);

            v.position = corigin + glm::vec2(m_characterSize.x,0.0);
            v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,0.0);
            guivertices.push_back(v);

            v.position = corigin + glm::vec2(m_characterSize.x,m_characterSize.y);
            v.uv       = uvoffset + glm::vec2(m_textureUvSize.x,m_textureUvSize.y);
            guivertices.push_back(v);

            v.position = corigin + glm::vec2(0.0,m_characterSize.y);
            v.uv       = uvoffset + glm::vec2(0.0,m_textureUvSize.y);
            guivertices.push_back(v);
        }

        m_vertices->upload(guivertices.size() * sizeof(GUIVertex), guivertices.data());
        m_vertexCount = guivertices.size();
    }

    void GUITextComponent::pushRenderProps()
    {
        HWRenderParameter rp;
        rp.name = "texture";
        rp.type = HWT_TEXTURE;
        rp.data.tx.tx = m_texture;        
        rp.data.tx.slot = 0;
        App::getHWAPI()->pushParam(rp);
    }

    void GUITextComponent::popRenderProps()
    {
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
}
