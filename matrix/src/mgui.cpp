#include <mgui.hpp>
#include <mapp.hpp>

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
    }

    void GUIItemComponent::renderComponent()
    {
        if(!m_material)
            return;

        pushRenderProps();

        HWRenderParameter rp;
        rp.name = "offset";
        rp.type = HWT_VECTOR3;
        rp.data.v3 = m_offset;
        App::getHWAPI()->pushParam(rp);
        rp.name = "scale";
        rp.type = HWT_VECTOR2;
        rp.data.v2 = m_size;
        App::getHWAPI()->pushParam(rp);

        App::getHWAPI()->gfxDrawArrays(HWAPI::HWPT_TRIANGLES, m_layout, m_vertexCount, m_material->getProgram());
        popRenderProps();
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
        setSize(m_imageTexture->getTextureSize());
        
        updateMesh();
    }

    void GUIImageComponent::createGuiMesh()
    {
        std::vector<GUIVertex> guivertices;
        GUIVertex v;

        v.position = glm::vec2(0.0,0.0);
        v.uv       = glm::vec2(0.0,0.0);
        guivertices.push_back(v);

        v.position = glm::vec2(1.0,0.0);
        v.uv       = glm::vec2(1.0,0.0);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,1.0);
        v.uv       = glm::vec2(0.0,1.0);
        guivertices.push_back(v);


        v.position = glm::vec2(1.0,0.0);
        v.uv       = glm::vec2(1.0,0.0);
        guivertices.push_back(v);


        v.position = glm::vec2(1.0,1.0);
        v.uv       = glm::vec2(1.0,1.0);
        guivertices.push_back(v);

        v.position = glm::vec2(0.0,1.0);
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
        rp.data.tx = m_imageTexture;
        App::getHWAPI()->pushParam(rp);
    }

    void GUIImageComponent::popRenderProps()
    {
        App::getHWAPI()->popParam();
    }
}