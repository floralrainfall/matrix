#include <mmaterial.hpp>
#include <mapp.hpp>
#include <mdev.hpp>
#include <cstring>

namespace mtx
{
    std::map<std::string, Material*> Material::m_materialCache;

    Material::Material(const char* file)
    {
        m_file = new ConfigFile(file);
        m_program = App::getHWAPI()->newProgram();

        std::string vf = m_file->getValue("glvertex");
        if(vf != "null")
            addShader(HWProgramReference::HWPST_VERTEX, vf);

        std::string ff = m_file->getValue("glfragment");
        if(ff != "null")
            addShader(HWProgramReference::HWPST_FRAGMENT, ff);

        std::string gf = m_file->getValue("glgeometry");
        if(gf != "null")
            addShader(HWProgramReference::HWPST_GEOMETRY, gf);

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
            fread(fshader, 1, 65535, fo);
            fclose(fo);
            
            DEV_MSG("adding shader %s", fpath.c_str());
            m_program->addShader(type, fshader);
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
    }
}