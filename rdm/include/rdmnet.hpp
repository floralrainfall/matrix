#include <mapp.hpp>
#include <mnet.hpp>
#include <mscene.hpp>
#include <mmodel.hpp>
#include <mmaterial.hpp>
#include <mphysics.hpp>
#include <glm/glm.hpp>
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
public:
    int m_lastPlayerId;
    mtx::SceneManager* m_scene; 
    std::map<int, RDMPlayer*> m_players;
    RDMPlayer* m_localPlayer;

    RDMNetListener(mtx::SceneManager* scene);

    virtual void onClientConnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onClientDisconnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onReceive(mtx::NetInterface* interface, mtx::NetClient* client, ENetPacket* packet);
    virtual void onFrame(mtx::NetInterface* interface);
};
