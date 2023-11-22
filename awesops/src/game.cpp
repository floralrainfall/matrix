#include <mapp.hpp>
#include <game.hpp>
#include <player.hpp>
#include <input.hpp>

namespace awesops
{
    Game globalGame;
    
    Game::Game()
    {

    }

    void Game::init(mtx::SceneManager* scene)
    {
	m_scene = scene;
	m_playerManager = new PlayerManager();
	m_inputManager = new InputManager();
	mtx::App::getHWAPI()->addListener(m_inputManager);

	mtx::SceneNode* mapNode = new mtx::SceneNode();
	m_map = new MapComponent();
	m_map->updateMesh();
	mapNode->addComponent(m_map);
	mapNode->setParent(m_scene->getRootNode());
    }

    void Game::tick()
    {
	m_playerManager->tick();
	m_inputManager->tick();
    }
}
