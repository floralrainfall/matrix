#include <mnet.hpp>
#include <mdev.hpp>

#define MAX_NETWORK_CLIENTS 255
#define SERVICE_TIME 1

namespace mtx
{
    void NetInterface::eventFrame()
    {
        if(App::getExecutionTime() < m_nextCheck)
            return;
        if(!m_online)
            return;
        m_nextCheck = App::getExecutionTime() + 0.016f;
        ENetEvent event;

        if(enet_host_service(m_peer, &event, SERVICE_TIME) > 0)
        {
            switch(event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                {
                    NetClient* c = getPeerClient(event.peer);
                    if(!c)
                    {
                        DEV_MSG("received packet from unknown peer");
                        break;
                    }
                    if(m_listener)
                        m_listener->onReceive(this, c, event.packet);
                    enet_packet_destroy(event.packet);
                }
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                {
                    NetClient* c = getPeerClient(event.peer);                    
                    char ad[32];
                    enet_address_get_host_ip(&event.peer->address, ad, sizeof(ad));
                    DEV_MSG("disconnect from %s:%i", ad, event.peer->address.port);
                    if(m_listener && c)
                        m_listener->onClientDisconnect(this, c);
                    if(!getServer())
                        m_online = false;
                    delete c;
                }
                break;
            case ENET_EVENT_TYPE_CONNECT:
                {
                    NetClient* c = new NetClient(event.peer);
                    char ad[32];
                    enet_address_get_host_ip(&event.peer->address, ad, sizeof(ad));
                    if(getServer())
                        DEV_MSG("new connection from %s:%i", ad, event.peer->address.port)
                    else
                        DEV_MSG("connection to %s:%i established", ad, event.peer->address.port)
                    enet_peer_ping_interval(event.peer, 16);
                    m_remoteClients.push_back(c);
                    if(m_listener)
                        m_listener->onClientConnect(this, c);
                }
                break;
            }
        }

        if(m_listener)
            m_listener->onFrame(this);
    }

    NetClient* NetInterface::getPeerClient(ENetPeer* peer)
    {
        for(NetClient* c : m_remoteClients)
        {
            if(c->getPeer() == peer)
                return c;
        }
        return NULL;
    }

    void NetInterface::setHostSettings(ENetHost* peer)
    {
        
    }

    NetServer::NetServer(ENetAddress address)
    {
        m_peer = enet_host_create(&address, MAX_NETWORK_CLIENTS, 2, 0, 0);
        setHostSettings(m_peer);
        m_server = true;
        m_online = true;
        if(!m_peer)
        {
            DEV_MSG("could not create enet server");
            m_online = false;
        }
        DEV_MSG("started server on port %i", address.port);
        m_listener = 0;
    }

    NetClient::NetClient()
    {
        m_peer = enet_host_create(NULL, 1, 2, 0, 0);
        setHostSettings(m_peer);
        m_server = false;
        m_online = false;
        if(!m_peer)
            DEV_MSG("could not create enet client");
        m_listener = 0;
    }

    NetClient::NetClient(ENetPeer* peer)
    {
        m_peer = 0;
        m_server = false;
        m_clientPeer = peer;
    }

    void NetClient::tryConnect(ENetAddress address)
    {
        m_clientPeer = enet_host_connect(m_peer, &address, 2, 0);
        if(m_clientPeer == NULL)
        {
            DEV_MSG("no available peers for initializing connection");
            m_online = false;
        }
        char ad[32];
        enet_address_get_host_ip(&address, ad, sizeof(ad));
        DEV_MSG("connecting to %s:%i", ad, address.port);
	m_online = true;
    }
}
