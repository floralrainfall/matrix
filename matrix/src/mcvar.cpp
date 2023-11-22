#pragma once
#include <mcvar.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

namespace mtx
{
    ConVarManager* App::conVarManager = NULL;
    
    ConVar::ConVar(std::string name,
		   std::string desc,
		   std::string initial,
		   ConVarChangeFunction change)
    {
	m_name = name;
	m_desc = desc;
	m_value = initial;
	m_changeFunction = change;

#ifdef CONVAR_ENABLE
	if(App::conVarManager)
	{
	    App::conVarManager->registerConVar(this);
	}
	else
	{
	    App::conVarManager = new ConVarManager();
	    App::conVarManager->registerConVar(this);
	}
#endif
    }

    int ConVar::getInt()
    {
	return std::stol(m_value);
    }

    double ConVar::getFloat()
    {
	return std::stod(m_value);
    }

    std::string ConVar::getString()
    {
	return m_value;
    }

    bool ConVar::getBool()
    {
	if(m_value == "0" || m_value == "")
	    return false;
	else
	    return true;
    }

    glm::vec2 ConVar::getVec2()
    {
	glm::vec4 v = getVec4();
	return glm::vec2(v.x, v.y);
    }

    glm::vec3 ConVar::getVec3()
    {
	glm::vec4 v = getVec4();
	return glm::vec3(v.x, v.y, v.z);
    }

    glm::vec4 ConVar::getVec4()
    {
	glm::vec4 v(0);
	size_t pos = 0;
	std::string s = m_value;
	std::string token;
	enum pos_t {
	    POSITION_X,
	    POSITION_Y,
	    POSITION_Z,
	    POSITION_W
	} *pos_e = (pos_t*)&pos;
	while((pos = s.find(" ")) != std::min(std::string::npos, (size_t)3)) {
	    std::string token = s.substr(0, pos);
	    switch(*pos_e)
	    {
	    case POSITION_X:
		v.x = std::stof(token);
		break;
	    case POSITION_Y:
		v.y = std::stof(token);
		break;
	    case POSITION_Z:
		v.z = std::stof(token);
		break;
	    case POSITION_W:
		v.w = std::stof(token);
		break;
	    }
	    s.erase(0, pos + 1);
	}
	return v;
    }

    void ConVar::setString(std::string s)
    {
	m_value = s;
    }

    void ConVar::setFloat(float f)
    {
	m_value = std::to_string(f);
	if(m_changeFunction)
	    m_changeFunction(this);
    }

    void ConVar::setInt(int i)
    {
	m_value = std::to_string(i);
    }

    ConVarManager::ConVarManager()
    {
	
    }

    ConVar* ConVarManager::getConVar(std::string name)
    {
	for(int i = 0; i < m_conVars.size(); i++)
	{
	    if(m_conVars[i]->getName() == name)
		return m_conVars[i];
	}
	DEV_SOFTWARN("could not find convar %s", name.c_str());
	return NULL;
    }

    void ConVarManager::registerConVar(ConVar* var)
    {
	if(this == 0)
	{
	    App::conVarManager = new ConVarManager();
	    App::conVarManager->registerConVar(var);
	    return;
	}
	m_conVars.push_back(var);
    }

    void ConVarManager::conVarCommand(std::string name,
				      std::string value)
    {
	ConVar* cvar = getConVar(name);
	if(!cvar)
	{
	    return;
	}
	cvar->setString(value);
    }

    void ConVarManager::listConVars()
    {
	for(int i = 0; i < m_conVars.size(); i++)
	{
	    INFO_MSG("'%s' = '%s', help: %s",
		    m_conVars[i]->getName().c_str(),
		    m_conVars[i]->getString().c_str(),
		    m_conVars[i]->getDesc().c_str());
	}
    }
}
