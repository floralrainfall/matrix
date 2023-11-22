#include <mmaterial.hpp>
#include <mapp.hpp>
#include <mdev.hpp>
#include <mmodel.hpp>
#include <format>
#include <cstring>

namespace mtx
{
    std::map<std::string, Material*> Material::m_materialCache;

    Material::Material(const char* file)
    {
	DEV_MSG("creating material %s, hwapi prefix: %s",
		file,
		App::getHWAPI()->getShaderPrefix().c_str());
        m_file = new ConfigFile(file);
        m_program = 0;
        Material* inheritedMaterial = NULL;
        
        if(m_file->getValue("reference") == "true")
            return;

        m_program = App::getHWAPI()->newProgram();

        std::string inherit = m_file->getValue("inherit");
        if(inherit != "null")
        {
            inheritedMaterial = Material::getMaterial(inherit.c_str());
            DEV_MSG("inherited material %s", inherit.c_str());
        }

	std::string prefix = App::getHWAPI()->getShaderPrefix();
	
        std::string vf = m_file->getValue((prefix + "vertex").c_str());
        if(vf != "null")
            addShader(HWProgramReference::HWPST_VERTEX, vf);
        else if(inheritedMaterial)
        {
            vf = inheritedMaterial->m_file->getValue((prefix + "vertex").c_str());
            if(vf != "null")
                addShader(HWProgramReference::HWPST_VERTEX, vf);
	    else
		DEV_SOFTWARN("inherited material has no vertex shader");
        }
	else
	    DEV_SOFTWARN("material has no vertex shader");

        std::string ff = m_file->getValue((prefix + "fragment").c_str());
        if(ff != "null")
            addShader(HWProgramReference::HWPST_FRAGMENT, ff);
        else if(inheritedMaterial)
        {
            ff = inheritedMaterial->m_file->getValue((prefix + "fragment").c_str());
            if(ff != "null")
                addShader(HWProgramReference::HWPST_FRAGMENT, ff);
	    else
		DEV_SOFTWARN("inherited material has no fragment shader");
        }
	else
	    DEV_SOFTWARN("material has no fragment shader");

        std::string gf = m_file->getValue((prefix + "geometry").c_str());
        if(gf != "null")
            addShader(HWProgramReference::HWPST_GEOMETRY, gf);
        else if(inheritedMaterial)
        {
            gf = inheritedMaterial->m_file->getValue((prefix + "geometry").c_str());
            if(gf != "null")
                addShader(HWProgramReference::HWPST_GEOMETRY, gf);
	    else
		DEV_MSG("inherited material has no geometry shader");
        }
	else
	    DEV_MSG("material has no geometry shader");

        m_program->link();
    }
    
    Material* Material::getMaterial(const char* name)
    {
        auto it = m_materialCache.find(name);
        if(it != m_materialCache.end())
        {
            return m_materialCache[name];
        }
        else
        {
            Material* newMaterial = new Material(name);
            m_materialCache[name] = newMaterial;
            return newMaterial;
        }
    }

    void Material::addShader(HWProgramReference::ShaderType type, std::string fpath)
    {
        std::FILE* fo = App::getFileSystem()->open(fpath.c_str());
        if(fo)
        {
            char fshader[65535];
            std::memset(fshader, 0, 65535);
            int read = fread(fshader, 1, 65535, fo);
            fclose(fo);
            
            DEV_MSG("adding shader %s", fpath.c_str());
            m_program->addShader(type, fshader, read);
        }
    }

    void Material::freeAllMaterials()
    {
        for(auto material : m_materialCache)
            delete material.second;
        m_materialCache.clear();
    }

    MaterialComponent::MaterialComponent(Material* material)
    {
        m_material = material;
	m_color = glm::vec4(1,1,1,1);
        m_kSpecular = 8;
    }

    void MaterialComponent::addShaderParams()
    {
        if(!m_material)
            return;

        HWRenderParameter rp;
        rp.name = "specular_k";
        rp.type = HWT_INT;
        rp.data.i = m_kSpecular;

	App::getHWAPI()->pushParam(rp);

        rp.name = "color";
        rp.type = HWT_VECTOR4;
        rp.data.v4 = m_color;

	App::getHWAPI()->pushParam(rp);
    }

    void MaterialComponent::updateFromModelComponent()
    {
        ModelComponent* model = (ModelComponent*)m_node->getComponent("ModelComponent");
        
        DEV_ASSERT(model);
        if(!model)
            return;
        
        ModelData* data = model->getModelData();

        DEV_ASSERT(data);
        if(!data)
            return;

        if(!data->m_modelScene->HasMaterials())
            return;

        int index = data->m_usedMesh->mMaterialIndex;

        DEV_ASSERT2(index, >, data->m_modelScene->mNumMaterials);
        aiMaterial* material = data->m_modelScene->mMaterials[index];

        DEV_ASSERT(material);
        if(!material)
            return;

        aiShadingMode mode;
        material->Get(AI_MATKEY_SHADING_MODEL, mode);
        std::string modelname = std::format("materials/sm/model{}.mmf", (int)mode);
        DEV_MSG("opening model %s", modelname.c_str());
        m_material = Material::getMaterial(modelname.c_str());
    }

    void MaterialComponent::popShaderParams()
    {
        if(!m_material)
            return;
        App::getHWAPI()->popParam();
	App::getHWAPI()->popParam();
    }
}
