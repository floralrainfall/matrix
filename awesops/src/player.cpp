#include <player.hpp>
#include <mdev.hpp>
#include <game.hpp>

namespace awesops
{
    BasePlayer::BasePlayer()
    {

    }

    void BasePlayer::tick()
    {

    }

    NetPlayer::NetPlayer() : BasePlayer()
    {

    }

    LocalPlayer::LocalPlayer() : BasePlayer()
    {
	m_inputManager = globalGame.getInputManager();
	m_cameraSettings.distance = 5.f;
	m_cameraSettings.angle1 = 45.f * (M_PI / 180.f);
	m_cameraSettings.angle2 = 0;
    }

    void LocalPlayer::tick()
    {
	mtx::SceneTransform& player_tf = m_playerNode->getTransform();
	glm::vec3 p1 = glm::vec3(
	    0,
	    sinf(m_cameraSettings.angle2) * m_cameraSettings.distance,
	    cosf(m_cameraSettings.angle2) * m_cameraSettings.distance);
	glm::vec3 p2 = glm::vec3(
	    sinf(m_cameraSettings.angle1) * m_cameraSettings.distance,
	    cosf(m_cameraSettings.angle1) * m_cameraSettings.distance,
	    0);
	glm::vec3 p = p1 + p2 + player_tf.getPosition();
	mtx::SceneTransform& camera_tf = m_cameraNode->getTransform();
	camera_tf.setPosition(p);
	camera_tf.setWorldMatrix(glm::lookAt(
				     p,
				     player_tf.getPosition(),
				     glm::vec3(0,0,1)
				     ));
	m_cameraSettings.angle1 = mtx::App::getExecutionTime() * (M_PI / 180.f);
    }

    AIPlayer::AIPlayer() : BasePlayer()
    {

    }

    PlayerManager::PlayerManager()
    {
	m_lastId = 0;
    }

    int PlayerManager::addPlayer(BasePlayer* player)
    {
	m_players[m_lastId++] = player;
	DEV_MSG("new player joining");
	return m_lastId - 1;
    }

    void PlayerManager::loadResources()
    {
	
    }

    void PlayerManager::tick()
    {
	for(auto p : m_players)
	{
	    p.second->tick();
	}
    }
}
