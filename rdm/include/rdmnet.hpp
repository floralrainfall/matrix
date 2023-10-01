#include <mapp.hpp>
#include <mnet.hpp>
#include <mscene.hpp>
#include <mmodel.hpp>
#include <mmaterial.hpp>
#include <glm/glm.hpp>
#include <map>

struct RDMPlayer
{
    mtx::SceneNode* node;
    glm::vec3 position;
    int playerid;
    mtx::NetClient* client;
};

class RDMNetListener : public mtx::NetEventListener
{
    int m_lastPlayerId;
    mtx::SceneManager* m_scene; 
    std::map<int, RDMPlayer*> m_players;
    RDMPlayer* m_localPlayer;
public:
    // scene should be NULL on server listener
    RDMNetListener(mtx::SceneManager* scene);

    virtual void onClientConnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onClientDisconnect(mtx::NetInterface* interface, mtx::NetClient* client);
    virtual void onReceive(mtx::NetInterface* interface, mtx::NetClient* client, ENetPacket* packet);
};