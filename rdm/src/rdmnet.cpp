#include <rdmnet.hpp>
#include <mdev.hpp>

enum PacketType
{
    RDMPAK_PLAYERINFO,
    RDMPAK_CURRENTPLAYERINFO,
    RDMPAK_DESTROYPLAYER,
    RDMPAK_MOTD,
};

union packetdata {
    struct {
        int playerid;
    } playerinfo;
    struct {
        char motd[128];

    } motd;
};

RDMNetListener::RDMNetListener(mtx::SceneManager* scene)
{
    m_scene = scene;
    m_lastPlayerId = 0;
    m_localPlayer = 0;
}

void RDMNetListener::onClientConnect(mtx::NetInterface* interface, mtx::NetClient* client)
{
    if(interface->getServer())
    {
        for(auto p : m_players)
        {
            ENetPacket* playerpacket = enet_packet_create(0, sizeof(packetdata::playerinfo) + 1, ENET_PACKET_FLAG_RELIABLE);
            playerpacket->data[0] = RDMPAK_PLAYERINFO;
            packetdata* pt = (packetdata*)(playerpacket->data + 1);
            pt->playerinfo.playerid = p.second->playerid;
            enet_peer_send(client->getPeer(), 0, playerpacket);
        }

        int id = m_lastPlayerId++;
        RDMPlayer* player = new RDMPlayer();
        player->playerid = id;
        player->client = client;
        client->setUserData((void*)player);
        m_players[id] = player;

        DEV_MSG("added new player %i", player->playerid);

        ENetPacket* playerpacket = enet_packet_create(0, sizeof(packetdata::playerinfo) + 1, ENET_PACKET_FLAG_RELIABLE);
        
        playerpacket->data[0] = RDMPAK_PLAYERINFO;
        packetdata* pt = (packetdata*)(playerpacket->data + 1);
        pt->playerinfo.playerid = player->playerid;
        enet_host_broadcast(interface->getHost(), 0, playerpacket);

        playerpacket = enet_packet_create(0, sizeof(packetdata::playerinfo) + 1, ENET_PACKET_FLAG_RELIABLE);
        playerpacket->data[0] = RDMPAK_CURRENTPLAYERINFO;
        pt = (packetdata*)(playerpacket->data + 1);
        pt->playerinfo.playerid = player->playerid;
        enet_peer_send(client->getPeer(), 0, playerpacket);

        ENetPacket* motdpacket = enet_packet_create(0, sizeof(packetdata::motd) + 1, ENET_PACKET_FLAG_RELIABLE);
        motdpacket->data[0] = RDMPAK_MOTD;
        pt = (packetdata*)(motdpacket->data + 1);
        strncpy(pt->motd.motd, "Welcome to the world of RDM", 128);
        enet_peer_send(client->getPeer(), 0, motdpacket);
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
            RDMPlayer* newplayer = new RDMPlayer();
            newplayer->playerid = dt->playerinfo.playerid;
            m_players[newplayer->playerid] = newplayer;
        }
        break;
    case RDMPAK_CURRENTPLAYERINFO:
        if(!interface->getServer())
        {
            DEV_MSG("authenticated as player id %i", dt->playerinfo.playerid);
            try{
                m_localPlayer = m_players.at(dt->playerinfo.playerid);
            } catch(std::exception e)
            {
                DEV_MSG("could not find local player");
            }
        }
        break;
    case RDMPAK_DESTROYPLAYER:
        if(!interface->getServer())
        {
            DEV_MSG("destroying player %i", dt->playerinfo.playerid);
            m_players.erase(dt->playerinfo.playerid);
        }
        break;
    case RDMPAK_MOTD:
        if(!interface->getServer())
        {
            DEV_MSG("MOTD: %s", dt->motd.motd);
        }
        break;
    default:
        DEV_MSG("unknown packet id %02x", (int)packettype);
        break;
    }
}
