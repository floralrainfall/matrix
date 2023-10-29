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
	/**
	 * Loads a new material from disk, or returns the material
	 * already loaded with name.
	 *
	 * @param name The name of the material to load.
	 */
        static Material* getMaterial(const char* name);
        static void freeAllMaterials();
        
        HWProgramReference* getProgram() { return m_program; }
    };

    class MaterialComponent : public SceneComponent
    {
        Material* m_material;

        int m_kSpecular;
	glm::vec4 m_color;
    public:
        MaterialComponent(Material* material = 0);
        void addShaderParams();
        void popShaderParams();
        void updateFromModelComponent();

	void setColor(glm::vec4 color) { m_color = color; };
        virtual std::string className() { return "MaterialComponent"; }
        void setMaterial(Material* material) { m_material = material; }
        Material* getMaterial() { return m_material; }
    };
}
