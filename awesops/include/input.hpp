#pragma once
#include <mhwabs.hpp>

namespace awesops
{
    class InputManager : public mtx::HWAPI::EventListener
    {
	bool m_shouldQuit;
	glm::vec3 m_move;
	glm::vec2 m_look;
    public:
	InputManager();

	glm::vec3 getMove() { return m_move; };
	glm::vec2 getLook() { return m_look; };
	bool getShouldQuit() { return m_shouldQuit; }

	void tick();
	
	virtual void onQuit();
	virtual void onKeyDown(int key);
	virtual void onKeyUp(int key);
	virtual void onMouseMoveRel(int x, int y, mtx::Window* window);
    };
}
