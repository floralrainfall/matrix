#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>

namespace mtx
{
    class ConVar;
    typedef std::function<void(ConVar*)> ConVarChangeFunction;

    class ConVar
    {
	std::string m_name;
	std::string m_desc;
	std::string m_value;
	ConVarChangeFunction m_changeFunction;
    public:
	ConVar(
	    std::string name,
	    std::string desc = "",
	    std::string initial = "",
	    ConVarChangeFunction change = 0);

	std::string getName() { return m_name; }
	std::string getDesc() { return m_desc; }

	int getInt();
	double getFloat();
	std::string getString();
	bool getBool();
	
	glm::vec2 getVec2();
	glm::vec3 getVec3();
	glm::vec4 getVec4();

	void setString(std::string s);
	void setFloat(float f);
	void setInt(int i);

	void setChangeFunction(ConVarChangeFunction f)
	{
	    m_changeFunction = f;
	}
    };
    
    class ConVarManager
    {
	std::vector<ConVar*> m_conVars;
    public:
	ConVarManager();
	
	ConVar* getConVar(std::string name);

	void conVarCommand(std::string name, std::string value);
	void registerConVar(ConVar* var);
	void listConVars();
    };
}
