#pragma once
#include <enet/enet.h>
#include <vector>

namespace mtx
{
    class NetClient;
    class NetServer;
    class NetInterface;
    class NetEventListener
    {
    public:
        virtual void onClientConnect(NetInterface* interface, NetClient* client) {};
        virtual void onClientDisconnect(NetInterface* interface, NetClient* client) {};
        virtual void onReceive(NetInterface* interface, NetClient* client, ENetPacket* packet) {};
        virtual void onFrame(NetInterface* interface) {};
    };

    class NetInterface
    {
        double m_nextCheck;
    protected:
        ENetHost* m_peer;
        NetEventListener* m_listener;
        std::vector<NetClient*> m_remoteClients;
        bool m_server;
        bool m_online;
        void* m_userData;
        void setHostSettings(ENetHost* peer);
    public:
        NetClient* getPeerClient(ENetPeer* peer);
        void setEventListener(NetEventListener* listener) { m_listener = listener; }
        void eventFrame();
        bool getServer() { return m_server; }
	bool getOnline() { return m_online; }

        ENetHost* getHost() { return m_peer; }
        void* getUserData() { return m_userData; }
        void setUserData(void* data) { m_userData = data; }
    };

    class NetServer : public NetInterface
    {
    public:
        // use App::newServer
        NetServer(ENetAddress address);
    };

    class NetClient : public NetInterface
    {
    protected:
        ENetPeer* m_clientPeer;
    public:
        ENetPeer* getPeer() { return m_clientPeer; }
        // use App::newClient
        NetClient();
        NetClient(ENetPeer* peer);

        void tryConnect(ENetAddress address);
    };
}
