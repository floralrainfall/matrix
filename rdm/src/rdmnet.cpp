#include <rdmnet.hpp>
#include <mdev.hpp>

enum PacketType
{
    RDMPAK_PLAYERINFO,
    RDMPAK_CURRENTPLAYERINFO
};

union packetdata {
    struct {
        int playerid;
    } playerinfo;
};

RDMNetListener::RDMNetListener(mtx::SceneManager* scene)
{
    m_scene = scene;
    m_lastPlayerId = 0;
    m_localPlayer.playerid = -1;
}

void RDMNetListener::onClientConnect(mtx::NetInterface* interface, mtx::NetClient* client)
{
    if(interface->getServer())
    {
        for(auto p : m_players)
        {
            ENetPacket* playerpacket = enet_packet_create(0, sizeof(packetdata) + 1, 0);
            playerpacket->data[0] = RDMPAK_PLAYERINFO;
            packetdata* pt = (packetdata*)(playerpacket->data + 1);
            pt->playerinfo.playerid = p.second->playerid;
            enet_peer_send(client->getPeer(), 0, playerpacket);
            enet_packet_destroy(playerpacket);
        }

        int id = m_lastPlayerId++;
        RDMPlayer* player = new RDMPlayer();
        player->playerid = id;
        player->client = client;
        client->setUserData((void*)player);
        m_players[id] = player;

        DEV_MSG("added new player %i", player->playerid);

        ENetPacket* playerpacket = enet_packet_create(0, sizeof(packetdata) + 1, 0);
        
        playerpacket->data[0] = RDMPAK_PLAYERINFO;
        packetdata* pt = (packetdata*)(playerpacket->data + 1);
        pt->playerinfo.playerid = player->playerid;
        enet_host_broadcast(interface->getHost(), 0, playerpacket);

        playerpacket->data[0] = RDMPAK_CURRENTPLAYERINFO;
        enet_peer_send(client->getPeer(), 0, playerpacket);
        enet_packet_destroy(playerpacket);
    }
}

void RDMNetListener::onClientDisconnect(mtx::NetInterface* interface, mtx::NetClient* client)
{
    RDMPlayer* player = (RDMPlayer*)client->getUserData();
    m_players.erase(player->playerid);
}

void RDMNetListener::onReceive(mtx::NetInterface* interface, mtx::NetClient* client, ENetPacket* packet)
{
    char packettype = packet->data[0];
    packetdata* dt = (packetdata*)(packet->data + 1);
    switch(packettype)
    {
    case RDMPAK_PLAYERINFO:
        if(!interface->getServer())
        {
            DEV_MSG("receiving player id %i", dt->playerinfo.playerid);
        }
        break;
    case RDMPAK_CURRENTPLAYERINFO:
        if(!interface->getServer())
        {
            DEV_MSG("authenticated as player id %i", dt->playerinfo.playerid);
            m_localPlayer.playerid = dt->playerinfo.playerid;
        }
        break;
    default:
        DEV_MSG("unknown packet id %02x", (int)packettype);
        break;
    }
}
