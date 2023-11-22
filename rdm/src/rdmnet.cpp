#include <rdmnet.hpp>
#include <mphysics.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

static mtx::Material* playerMaterial;
static mtx::ModelData* playerModel;

enum PacketType
{
    RDMPAK_PLAYERINFO,
    RDMPAK_CURRENTPLAYERINFO,
    RDMPAK_DESTROYPLAYER,
    RDMPAK_PLAYERPOSITION,
    RDMPAK_CHANGELEVEL,
    RDMPAK_MOTD,
};

union packetdata {
    struct {
        int playerid;
	double tickrate;
    } playerinfo;
    struct {
        char motd[128];
    } motd;
    struct {
	char level[128];
    } changelevel;
    struct {
        int playerid;
        glm::vec3 position; // delta position
        glm::quat direction;
	bool delta;
    } playerposition;
};

RDMNetListener::RDMNetListener(mtx::SceneManager* scene)
{
    m_scene = scene;
    m_lastPlayerId = 0;
    m_localPlayer = 0;
    m_currentMap = 0;
    m_mapComponent = 0;
    m_ready = false;
    m_pendingNewMap = false;
}

void RDMNetListener::loadResources()
{ 
    playerMaterial =
	mtx::Material::getMaterial("materials/diffuse.mmf");
    playerModel = mtx::ModelData::loadCached("models/ball.obj");   
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

        player->node = new mtx::SceneNode();
        player->node->setParent(m_scene->getRootNode());

	player->playerbody = new mtx::RigidBody(
	    new btCapsuleShape(16.f,32.f),
	    1.f
	);
	player->node->addComponent(player->playerbody);

	player->position = mtx::BulletHelpers::v3FromBullet(player->playerbody->getTransform().getOrigin());

        DEV_MSG("added new player %i", player->playerid);

        ENetPacket* playerpacket = enet_packet_create(0, sizeof(packetdata::playerinfo) + 1, ENET_PACKET_FLAG_RELIABLE);
        
        playerpacket->data[0] = RDMPAK_PLAYERINFO;
        packetdata* pt = (packetdata*)(playerpacket->data + 1);
        pt->playerinfo.playerid = player->playerid;
	pt->playerinfo.tickrate = m_scene->getApp()->getTickRate();
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
	
	DEV_ASSERT(m_currentMap);
    
        ENetPacket* levelpacket = enet_packet_create(0, sizeof(packetdata::changelevel) + 1, ENET_PACKET_FLAG_RELIABLE);
        levelpacket->data[0] = RDMPAK_CHANGELEVEL;
        pt = (packetdata*)(levelpacket->data + 1);
        strncpy(pt->changelevel.level, m_currentMap->getName().c_str(), 128);
        enet_peer_send(client->getPeer(), 0, levelpacket);
    }
}

void RDMNetListener::onClientDisconnect(mtx::NetInterface* interface, mtx::NetClient* client)
{
    if(interface->getServer())
    {
	RDMPlayer* player = (RDMPlayer*)client->getUserData();
	m_players.erase(player->playerid);
    }
    else
    {
	DEV_SOFTWARN("client disconnected!");
    }
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
            newplayer->node = new mtx::SceneNode();
            newplayer->node->setParent(m_scene->getRootNode());

            mtx::MaterialComponent* playermat =
		new mtx::MaterialComponent(
		    playerMaterial
		    );
            mtx::ModelComponent* playermdl =
		new mtx::ModelComponent();
	    playermdl->setModelData(playerModel);

            newplayer->node->addComponent(playermdl);
            newplayer->node->addComponent(playermat);

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
    case RDMPAK_CHANGELEVEL:
	if(!interface->getServer())
	{
	    m_mapMutex.lock();
	    INFO_MSG("changing level to %s", dt->changelevel.level);

	    m_currentMap = new mtx::BSPFile(dt->changelevel.level); // we have to wait
	    INFO_MSG("loaded map");
	    m_pendingNewMap = true;
	    m_ready = true;
	    m_mapMutex.unlock();
	}
	break;
    case RDMPAK_MOTD:
        if(!interface->getServer())
        {
            INFO_MSG("MOTD: %s", dt->motd.motd);
        }
        break;
    case RDMPAK_PLAYERPOSITION:
        if(interface->getServer())
        {
            RDMPlayer* player = (RDMPlayer*)client->getUserData();
	    mtx::RigidBody* body =
		(mtx::RigidBody*)player->node->getComponent("RigidBody");

	    btTransform tf = body->getTransform();
	    player->position = mtx::BulletHelpers::v3FromBullet(tf.getOrigin());

	    /*
	      float dist = glm::distance(player->position,dt->playerposition.position);
	      if(dist < 10.0)
	      {
	        DEV_MSG("player %i is going too fast (%fu in 1 upd pak)",
	        player->playerid, dist);
	      break;
	      }
	    */
	    
	    if(dt->playerposition.delta)
		player->position += dt->playerposition.position;
	    else
		player->position = dt->playerposition.position;
	    if(player->node)
	    {
		tf.setOrigin(mtx::BulletHelpers::v3ToBullet(player->position));
		body->setTransform(tf);
	    }
	    
            player->direction = dt->playerposition.direction;
            player->position_dirty = true;
        }
        else
        {
            RDMPlayer* player = (RDMPlayer*)m_players[dt->playerposition.playerid];
            player->position = dt->playerposition.position;
            player->direction = dt->playerposition.direction;
            player->position_dirty = false;

            if(player->node)
            {
                mtx::SceneTransform& tf = player->node->getTransform();
                tf.setPosition(player->position);
                tf.setRotation(player->direction);
            }
        }
        break;
    default:
        DEV_MSG("unknown packet id %02x", (int)packettype);
        break;
    }
}

void RDMNetListener::onFrame(mtx::NetInterface* interface)
{
    if(interface->getServer())
    {
        for(auto p : m_players)
        {
	    btTransform bodytf = p.second->playerbody->getTransform();
	    if(glm::distance(p.second->last_upd_position,
			     mtx::BulletHelpers::v3FromBullet(bodytf.getOrigin()))
	       != 0)
		p.second->position_dirty = true;
            if(p.second->position_dirty)
            {
                ENetPacket* packet = enet_packet_create(0, sizeof(packetdata::playerposition) + 1, 0);
                packet->data[0] = RDMPAK_PLAYERPOSITION;
                packetdata* pd = (packetdata*)(packet->data + 1);

		pd->playerposition.position = p.second->position;
		pd->playerposition.delta = false;
		pd->playerposition.direction = p.second->direction;
                pd->playerposition.playerid = p.first;
                // enet_peer_send(((mtx::NetClient*)interface)->getPeer(), 0, packet);
                enet_host_broadcast(interface->getHost(), 0, packet);
                p.second->position_dirty = false;
            }
        }
    }   
    else
    {
        if(!m_localPlayer)
            return;

        if(m_localPlayer->position_dirty)
        {
            ENetPacket* packet = enet_packet_create(0,
						    sizeof(packetdata::playerposition) + 1, 0);
            packet->data[0] = RDMPAK_PLAYERPOSITION;
            packetdata* pd = (packetdata*)(packet->data + 1);
            pd->playerposition.position =
		m_localPlayer->last_upd_position -
		m_localPlayer->position;
	    m_localPlayer->last_upd_position = m_localPlayer->position;
	    pd->playerposition.delta = true;
            pd->playerposition.playerid = m_localPlayer->playerid;
            pd->playerposition.direction = m_localPlayer->direction;
            enet_peer_send(((mtx::NetClient*)interface)->getPeer(), 0, packet);
            m_localPlayer->position_dirty = false;
        }
    }
}
