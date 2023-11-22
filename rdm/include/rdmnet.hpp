#include <mapp.hpp>
#include <mnet.hpp>
#include <mscene.hpp>
#include <mmodel.hpp>
#include <mmaterial.hpp>
#include <mphysics.hpp>
#include <mbsp.hpp>
#include <glm/glm.hpp>
#include <mutex>
#include <map>

struct RDMPlayer
{
    mtx::SceneNode* node;
    glm::vec3 position;
    glm::vec3 last_upd_position;
    glm::quat direction;
    bool position_dirty;
    int playerid;
    mtx::NetClient* client;
    mtx::RigidBody* playerbody;
};

class RDMNetListener : public mtx::NetEventListener
{
    void updatePlayerPhysicsPosition(RDMPlayer* player);
    bool m_ready;
    mtx::BSPComponent* m_mapComponent;
public:
    std::mutex m_mapMutex;
    mtx::BSPFile* m_currentMap;
    bool m_pendingNewMap;
    int m_lastPlayerId;
    mtx::SceneManager* m_scene; 
    std::map<int, RDMPlayer*> m_players;
    RDMPlayer* m_localPlayer;

    RDMNetListener(mtx::SceneManager* scene);
    void loadResources();
    
    virtual void onClientConnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onClientDisconnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onReceive(mtx::NetInterface* interface, mtx::NetClient* client, ENetPacket* packet);
    virtual void onFrame(mtx::NetInterface* interface);

    bool getGameReady() { return m_ready; };
};
