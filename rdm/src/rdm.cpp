#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>
#include <mwweb.hpp>
#include <mbsp.hpp>
#include <mphysics.hpp>
#include <mwweb.hpp>
#include <mwuser.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>

#include "rdmnet.hpp"

mtx::ConVar rdm_launch_mode = mtx::ConVar("rdm_launch_mode",
					  "Launch mode of RDM",
					  "client");

enum RDMLaunchMode
{
    LAUNCH_SERVER,
    LAUNCH_HOST,
    LAUNCH_CLIENT,
};

class RDMOcclusionTester : public mtx::SceneManager::SceneOcclusionTester {
    mtx::BSPFile* m_bsp;
public:
    RDMOcclusionTester(mtx::BSPFile* bsp)
    {
        m_bsp = bsp;
    }

    virtual bool test(mtx::SceneTransform a, mtx::SceneTransform b)
    {
        return m_bsp->canSee(a.getPosition(), b.getPosition());
    }
};

class RDMListener : public mtx::HWAPI::EventListener
{
public:
    bool keysDown[1024];
    mtx::App* app;
    float xrel;
    float yrel;
    bool lockmouse;

    RDMListener(mtx::App* app)
    {
        this->app = app;
        lockmouse = true;
    }

    virtual void onWindowClose(mtx::Window* window)
    {
        DEV_MSG("closing window %p", window);
        window->setShouldClose(true);
    }

    virtual void onQuit()
    {
        app->setAppRunning(false);
    }

    virtual void onKeyDown(int key)
    {
        if(key > sizeof(keysDown))
            return;
        keysDown[key] = true;
    }

    virtual void onKeyUp(int key)
    {
        if(key > sizeof(keysDown))
            return;
        keysDown[key] = false;
    }

    virtual void onMouseMoveRel(int x, int y, mtx::Window* window)
    {
        xrel = x;
        yrel = y;            
    }

    virtual void onWindowSize(int w, int h, mtx::Window* window)
    {
        std::vector<mtx::Viewport*> viewports = window->getViewports();
        for(mtx::Viewport* viewport : viewports)
            viewport->setViewport(glm::vec4(0,0,w,h));
    }
};

class RDMApp : public mtx::App
{
public:
    mtx::SceneManager* m_uiSceneManager;
    mtx::Viewport* m_viewport;
    mtx::Viewport* m_uiViewport;
    mtx::Window* m_window;
    mtx::SceneNode* cameraNode;
    RDMListener* m_eventListener;
    RDMNetListener* m_netListenerServer;
    RDMNetListener* m_netListenerClient;
    mtx::GUITextComponent* m_guiShipStatus;
    mtx::GUIImageComponent* m_guiBackgroundImage;
    mtx::PhysicsWorld* m_physworld;
    mtx::BSPFile* bsp;
    mtx::BSPComponent* bspmodel;
    mtx::RigidBody* ballBody;
    mtx::world::WebService* m_webService;

    mtx::NetClient* client;
    mtx::SceneManager* m_clientSceneManager;
    mtx::NetServer* server;
    mtx::SceneManager* m_serverSceneManager;

    RDMLaunchMode launchMode;

    int menu_selectedItem;

    float m_cameraYaw;
    float m_cameraPitch;

    glm::vec3 cameraps;
    glm::vec3 velocity;

    RDMApp(int argc, char** argv) : App(argc, argv)
    {
	menu_selectedItem = 0;
    }
    
    virtual void init() {
	std::string launch_mode =
	    rdm_launch_mode.getString();
	if(launch_mode == "server")
	    launchMode = LAUNCH_SERVER;
	else
	    launchMode = LAUNCH_CLIENT;

	m_serverSceneManager = new mtx::SceneManager(this);
        m_clientSceneManager = new mtx::SceneManager(this);
	m_webService = new mtx::world::WebService();
	bsp = 0;
	velocity = glm::vec3(0);

	getScheduler()->setPaused(true);
	
        ENetAddress hostaddress;
        ENetAddress cliaddress;
        switch(launchMode)
        {
        case LAUNCH_SERVER:
            hostaddress.host = ENET_HOST_ANY;
            hostaddress.port = 7936;
            server = newServer(hostaddress);


            m_netListenerServer = new
		RDMNetListener(m_serverSceneManager);

	    bsp = new mtx::BSPFile("rdm/rdm/maps/ffa_me2.bsp");

	    m_netListenerServer->m_currentMap =
		bsp;
            server->setEventListener(m_netListenerServer);

	    getScheduler()->setPaused(false);
            break;
        case LAUNCH_HOST:
            hostaddress.host = ENET_HOST_ANY;
            hostaddress.port = 7936;
            server = newServer(hostaddress);

            m_netListenerServer = new RDMNetListener(m_serverSceneManager);
            server->setEventListener(m_netListenerServer);

	    getScheduler()->setPaused(false);
        case LAUNCH_CLIENT:
            m_viewport = new mtx::Viewport(640, 480);
            m_viewport->setClearColor(glm::vec4(0.4,0.4,0.4,1));
            m_viewport->setNearDistance(1.f);
            m_viewport->setFarDistance(65535.f);
            
            m_window = newWindow(m_viewport, mtx::HWAPI::HWWT_NORMAL_RESIZABLE);
            m_window->setTitle("RDM alpha");

            client = newClient();
            m_netListenerClient = new RDMNetListener(m_clientSceneManager);
            client->setEventListener(m_netListenerClient);

	    if(m_webService->getCurrentUser())
		m_webService->getCurrentUser()->loadWebContent();
            break;
        }

        m_physworld = new mtx::PhysicsWorld();
        m_serverSceneManager->getRootNode()->addComponent(m_physworld);
	if(bsp)
	    bsp->addToPhysicsWorld(m_physworld);

        m_eventListener = new RDMListener(this);
        App::getHWAPI()->addListener(m_eventListener);

        ballBody = 0;
        /* for(int i = 0; i < 100; i++)
        {
            mtx::SceneNode* ballNode = new mtx::SceneNode();
            ballNode->setParent(m_sceneManager->getRootNode());
            mtx::ModelComponent* ballModel = new mtx::ModelComponent("models/ball.obj");
            mtx::MaterialComponent* ballMaterial = new mtx::MaterialComponent(mtx::Material::getMaterial("materials/diffuse.mmf"));
            mtx::RigidBody* _ballBody = new mtx::RigidBody(new btSphereShape(16.f), 1.f);
            ballNode->addComponent(ballModel);
            ballNode->addComponent(ballMaterial);
            ballNode->addComponent(_ballBody);
            _ballBody->addToWorld(m_physworld);
            btTransform balltf;
            balltf.setOrigin(btVector3(0,0,255));
            _ballBody->setTransform(balltf);
            if(!ballBody)
                ballBody = _ballBody;
        } */
	
        cameraps = glm::vec3(0,0,0);

        // TODO: RDMOcclusionTester (and thus the BSP testing solution) is too slow at the moment
        // getting rid of it saves about ~10ms in Dt
        //m_sceneManager->setOcclusionTester(new RDMOcclusionTester(bsp));

        m_cameraYaw = 0.f;
        m_cameraPitch = 0.f;
    }

    virtual void initGfx() {
	App::getHWAPI()->loadResources();
	
	m_netListenerClient->loadResources();
        m_uiSceneManager = new mtx::SceneManager(this);
        // m_window->getHwWindow()->setGrab(true);

        m_uiViewport = new mtx::Viewport(640, 480);
        m_uiViewport->setClearColor(glm::vec4(0));

	mtx::ViewportSettings& uiVpSettings = m_uiViewport->getSettings();
	uiVpSettings.enableDepthTest = false;

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_clientSceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        mtx::SceneNode* healthNode = new mtx::SceneNode();
        healthNode->setParent(m_uiSceneManager->getRootNode());
        mtx::ModelComponent* healthModel = new mtx::ModelComponent("models/cross.obj");
        mtx::MaterialComponent* healthMaterial =
	    new mtx::MaterialComponent(
		mtx::Material::getMaterial("materials/diffuse.mmf"));       
	healthMaterial->setColor(glm::vec4(1,0,0,1));
        healthNode->addComponent(healthModel);
        healthNode->addComponent(healthMaterial);

        mtx::SceneNode* uiCameraNode = new mtx::SceneNode();
        uiCameraNode->setParent(m_uiSceneManager->getRootNode());
        mtx::SceneTransform& uiCameraTf = uiCameraNode->getTransform();
        uiCameraTf.setWorldMatrix(glm::lookAt(
            glm::vec3(8,0,0),
            glm::vec3(0,0,0),
            glm::vec3(0,1,0)            
        ));
        m_uiViewport->setCameraNode(uiCameraNode);
        m_window->addViewport(m_uiViewport);

        mtx::SceneTransform& healthTf = healthNode->getTransform();
        healthTf.setPosition(glm::vec3(-8,-8,10));

        mtx::SceneNode* guiNode = new mtx::SceneNode();
        m_guiShipStatus = new mtx::GUITextComponent();
        m_guiShipStatus->setText("RDM alpha");
        m_guiShipStatus->setFont(App::getHWAPI( )->loadCachedTexture("textures/font.png"));
        m_guiShipStatus->setOffset(glm::vec3(0.0, 480.0 - 16.0, 0.1));
        m_guiShipStatus->setCharacterSize(glm::ivec2(8,16));
        guiNode->setParent(m_uiSceneManager->getRootNode());

        mtx::GUIImageComponent* logoImage = new mtx::GUIImageComponent("textures/entropy_interactive_small.png");
	logoImage->setOffset(glm::vec3(0.0,0.0,0.1));

	m_guiBackgroundImage = new
	    mtx::GUIImageComponent("textures/rdm/background.png");
	m_guiBackgroundImage->setOffset(glm::vec3(0.0,0.0,0.9));
	m_guiBackgroundImage->setAbsoluteSize(true);
	
	guiNode->addComponent(m_guiBackgroundImage);
        guiNode->addComponent(logoImage);
        guiNode->addComponent(m_guiShipStatus);
	
        m_uiSceneManager->setSunDirection(glm::vec3(1,1,1));

    	App::console->initGfx(m_uiSceneManager, m_uiViewport);

	mtx::SceneNode* bspnode = new mtx::SceneNode();
	bspmodel = new mtx::BSPComponent(NULL);
	bspnode->addComponent(bspmodel);
	bspnode->setParent(m_clientSceneManager->getRootNode());
    }
    
    virtual void tickGfx()
    {
	App::console->tickGfx();
	
        mtx::SceneTransform& cameratf = cameraNode->getTransform();
        cameratf.setPosition(cameraps);


	if(m_netListenerClient->getGameReady())
	{
	    m_cameraYaw += m_eventListener->xrel * getDeltaTime();
	    m_cameraPitch += m_eventListener->yrel * getDeltaTime();
	    m_eventListener->xrel = 0.f;
	    m_eventListener->yrel = 0.f;

	    float clock = 1.5f;
	    // m_cameraPitch = SDL_clamp(m_cameraPitch, -clock, clock);
	}
	
        glm::quat qx = glm::angleAxis(m_cameraPitch, glm::vec3(1,0,0));
        glm::quat qy = glm::angleAxis(0.f, glm::vec3(0,1,0));
        glm::quat qz = glm::angleAxis(m_cameraYaw, glm::vec3(0,0,1));
        glm::quat qf = qx * qy * qz;
        glm::vec3 front = glm::vec3(0, 1, 0) * qf;
        glm::vec3 right = glm::vec3(1, 0, 0) * qf;

	//if(m_netListenerClient->getGameReady())
	//{
	    float movespeed = 64.f;
	    static bool debounce;
	    if(m_eventListener->keysDown['w'])
		velocity += movespeed * front * (float)getDeltaTime();
	    if(m_eventListener->keysDown['s'])
		velocity -= movespeed * front * (float)getDeltaTime();
	    if(m_eventListener->keysDown['a'])
		velocity -= movespeed * right * (float)getDeltaTime();
	    if(m_eventListener->keysDown['d'])
		velocity += movespeed * right * (float)getDeltaTime();
	    velocity *= 0.99;
	    cameraps += velocity * (float)getDeltaTime();
	    cameratf.setWorldMatrix(glm::lookAt(
					cameraps,
					front + cameraps,
					glm::vec3(0,0,1)));
	    //}
	
        if(m_netListenerClient->m_localPlayer)
        {
            if(glm::distance(cameraps,m_netListenerClient->m_localPlayer->position) > 0.1 ||
               glm::dot(qf, m_netListenerClient->m_localPlayer->direction) < 0.9)
            {
                m_netListenerClient->m_localPlayer->position_dirty = true;
                m_netListenerClient->m_localPlayer->position = cameraps;
                m_netListenerClient->m_localPlayer->direction = qf;
            }
            else
            {
                if(glm::distance(cameraps,m_netListenerClient->m_localPlayer->position) > 0.5) // dont update if we're really close because then you'll move slow if your dt is too small
                    cameraps = m_netListenerClient->m_localPlayer->position;
            }
        }

	if(m_netListenerClient->getGameReady())
	{	    
	    char statustx[1024];
	    snprintf(statustx, 1024, "RDM alpha\n"
		     "vertices: %i, calls: %i\n"
		     "%f, %f, %f %f %f, %f %f %f\n"
		     "Dt=%f T=%f Ping=%ims\n"
		     "Net Loss=%i, Sent=%i, Lost=%i\n"
		     "PDt=%f",
		     App::getHWAPI()->getDrawnVertices(), App::getHWAPI()->getDrawCalls(),
		     front.x, front.y, front.z, m_cameraYaw, m_cameraPitch, cameraps.x, cameraps.y, cameraps.z,
		     getDeltaTime(), getExecutionTime(), client->getPeer()->lastRoundTripTime,
		     client->getPeer()->packetLoss, client->getPeer()->packetsSent, client->getPeer()->packetsLost,
		     m_physworld->getDeltaTime());
	    
	    m_guiShipStatus->setText(statustx);
	    
	    m_guiBackgroundImage->setVisible(false);
	}
	else
	{
	    char statustx[1024];

	    glm::vec4 uiv = m_uiViewport->getViewport();
	    m_guiBackgroundImage->setVisible(true);
	    m_guiBackgroundImage->setSize(glm::vec2(uiv.z,
						    uiv.w));
					  
	    if(menu_selectedItem == 0)
	    {
		if(m_eventListener->keysDown['1'])
		    menu_selectedItem = 1;
		if(m_eventListener->keysDown['2'])
		    menu_selectedItem = 2;
		if(m_eventListener->keysDown['3'])
		    menu_selectedItem = 3;		
		snprintf(statustx, 1024, "Ryelow Deathmatch\n"
			 "1: Connect to Game\n"
			 "2: Create a Game\n"
			 "3: Quit\n");
	    }
	    else if(menu_selectedItem == 1)
	    {
		snprintf(statustx, 1024, "Connect\n"
			 "x: Exit to main menu\n"
			 "c: Connect to 127.0.0.1\n");
		if(m_eventListener->keysDown['x'])
		    menu_selectedItem = 0;
		if(m_eventListener->keysDown['c'])
		{
		    ENetAddress cliaddress;
		    
		    enet_address_set_host_ip(&cliaddress, "127.0.0.1");
		    cliaddress.port = 7936;

		    m_guiBackgroundImage->setImageTexture(
			App::getHWAPI()->loadCachedTexture("textures/rdm/background_plswait.png")
			);
		    
		    client->tryConnect(cliaddress);
		    getScheduler()->setPaused(false);

		    menu_selectedItem = -1;

		    mtx::App::console->setVisible(true);
		}
	    }
	    else if(menu_selectedItem == 0)
	    {

	    }
	    m_guiShipStatus->setText(statustx);
	}

	double time = getExecutionTime();
	m_guiShipStatus->setColor(glm::vec4(
				      (sinf(time+(M_PI * (1.0/3.0)))+1.0)/2.0,
				      (sinf(time+(M_PI * (2.0/3.0)))+1.0)/2.0,
				      (sinf(time+(M_PI * (3.0/3.0)))+1.0)/2.0,
				      1.0));

	if(client->getConnected())
	{
	    // we need to load bsps in the gfx/main thread so the
	    // assets dont create themselves on Tick (which isnt
	    // allowed)

	    if(m_netListenerClient->m_pendingNewMap)
	    {
		m_netListenerClient->m_mapMutex.lock();
		INFO_MSG("loading map resources");
		m_netListenerClient->m_currentMap->initGfx();
		INFO_MSG("loaded map resources");
		if(bsp)
		{
		    DEV_MSG("unloading old map");
		    delete bsp; // delete old bsp
		}
		bspmodel->setBSP(m_netListenerClient->m_currentMap);
		bsp = m_netListenerClient->m_currentMap;
		m_netListenerClient->m_pendingNewMap = false;

		m_guiBackgroundImage->setColor(glm::vec4(0.25,0.25,0.25,0.5));
		m_netListenerClient->m_mapMutex.unlock();

		mtx::App::console->setVisible(false);
	    }
	}
    }

    virtual void tick() {
        if(launchMode == LAUNCH_SERVER)
        {
            char statustx[1024];
            snprintf(statustx, 1024, ", Players Online: %i", m_netListenerServer->m_players.size());
            setHeadlessStatus(statustx);
        }
    } 
};



#ifndef COMPILE_EXECUTABLE
static mtx::App* __mainRoutine(int argc, char** argv)
{
    return new RDMApp(argc, argv);
}

extern "C" {
    mtx::HLResponse* __MatrixMain(mtx::HLContext context)
    {
	mtx::HLResponse* rsp =
	    (mtx::HLResponse*)malloc(sizeof(mtx::HLResponse));
	switch(context.request)
	{
	case mtx::HLR_GET_MAIN_ROUTINE:
	    rsp->status = mtx::HLS_SUCCESS;
	    rsp->data.mainRoutine.routine = __mainRoutine;
	    break;
	default:
	    rsp->status = mtx::HLS_UNSUPPORTED_REQUEST;
	    break;
	}
	return rsp;
    }
}
#else
int main(int argc, char** argv)
{
    RDMApp app = RDMApp(argc, argv);
    return app.main();
}
#endif
