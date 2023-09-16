#pragma once
#include <mcfg.hpp>
#include <mhwabs.hpp>
#include <mscene.hpp>
#include <map>
#include <string>

namespace mtx
{
    class Material
    {
        static std::map<std::string, Material*> m_materialCache;

        ConfigFile* m_file;
        HWProgramReference* m_program;

        void addShader(HWProgramReference::ShaderType type, std::string fpath);
        Material(const char* definitionFile);
    public:
        static Material* getMaterial(const char* name);
        static void freeAllMaterials();
        
        HWProgramReference* getProgram() { return m_program; }
    };

    class MaterialComponent : public SceneComponent
    {
        Material* m_material;

        int m_kSpecular;
    public:
        MaterialComponent(Material* material = 0);
        void addShaderParams();
        void popShaderParams();
        void updateFromModelComponent();

        virtual std::string className() { return "MaterialComponent"; }
        void setMaterial(Material* material) { m_material = material; }
        Material* getMaterial() { return m_material; }
    };
}