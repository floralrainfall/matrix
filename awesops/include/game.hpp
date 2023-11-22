#pragma once
#include <mscene.hpp>
#include <map.hpp>

namespace awesops
{
    class PlayerManager;
    class InputManager;
    
    class Game
    {
	mtx::SceneManager* m_scene;
	PlayerManager* m_playerManager;
	InputManager* m_inputManager;

	MapComponent* m_map;
    public:
	Game();

	void init(mtx::SceneManager* scene);

	PlayerManager* getPlayerManager()
	{
	    return m_playerManager;
	}

	InputManager* getInputManager()
	{
	    return m_inputManager;
	}

	void tick();
    };

    extern Game globalGame;
}
