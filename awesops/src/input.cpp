#include <input.hpp>

namespace awesops
{
    InputManager::InputManager()
    {
	m_shouldQuit = false;
    }

    void InputManager::onQuit()
    {
	m_shouldQuit = true;
    }

    void InputManager::tick()
    {
	m_look = glm::vec2(0.0);
	m_move = glm::vec3(0.0);
    }

    void InputManager::onKeyDown(int key)
    {
	if(key == 'w')
	{
	    
	}
    }

    void InputManager::onKeyUp(int key)
    {

    }

    void InputManager::onMouseMoveRel(int x, int y, mtx::Window* window)
    {
	
    }
}
