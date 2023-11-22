#pragma once
#include <mscene.hpp>
#include <map>
#include <input.hpp>

namespace awesops
{
    class BasePlayer
    {
    protected:
	mtx::SceneNode* m_playerNode;
	int m_playerId;
    public:
	BasePlayer();

	void setSceneNode(mtx::SceneNode* node)
	{
	    m_playerNode = node;
	}
	void setPlayerId(int id)
	{
	    m_playerId = id;
	}

	virtual void tick();
    };

    struct CameraControl
    {
	float distance;
	float angle1;
	float angle2;
    };

    class LocalPlayer : public BasePlayer
    {
	InputManager* m_inputManager;
	CameraControl m_cameraSettings;
	mtx::SceneNode* m_cameraNode;
    public:
	LocalPlayer();

	CameraControl& getCameraSettings()
	{
	    return m_cameraSettings;
	};
	
	void setCameraNode(mtx::SceneNode* camera)
	{
	    m_cameraNode = camera;
	};

	virtual void tick();
    };
    
    class NetPlayer : public BasePlayer
    {
    public:
	NetPlayer();	
    };

    class AIPlayer : public BasePlayer
    {
    public:
	AIPlayer();
    };

    class PlayerManager
    {
	std::map<int, BasePlayer*> m_players;
	int m_lastId;
    public:
	PlayerManager();

	void loadResources();
	int addPlayer(BasePlayer* player);
	BasePlayer* getPlayer(int id) { return m_players[id]; }
	void tick();
    };
};
