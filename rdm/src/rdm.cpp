#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>
#include <mwweb.hpp>
#include <mbsp.hpp>
#include <mphysics.hpp>
#include <hw/msdl.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>

#include "rdmnet.hpp"

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
    mtx::SceneManager* m_sceneManager;
    mtx::SceneManager* m_playerSceneManager;
    mtx::SceneManager* m_uiSceneManager;
    mtx::Viewport* m_viewport;
    mtx::Viewport* m_uiViewport;
    mtx::Window* m_window;
    mtx::SceneNode* cameraNode;
    RDMListener* m_eventListener;
    mtx::GUITextComponent* m_guiShipStatus;
    mtx::PhysicsWorld* m_physworld;
    mtx::BSPFile* bsp;
    mtx::RigidBody* ballBody;

    mtx::NetClient* client;
    mtx::NetServer* server;

    float m_cameraYaw;
    float m_cameraPitch;

    glm::vec3 cameraps;
    virtual void init() {
        m_sceneManager = new mtx::SceneManager(this);
        m_uiSceneManager = new mtx::SceneManager(this);
        m_playerSceneManager = new mtx::SceneManager(this);

        ENetAddress hostaddress;
        hostaddress.host = ENET_HOST_ANY;
        hostaddress.port = 7936;
        server = newServer(hostaddress);
        server->setEventListener(new RDMNetListener(NULL));
        ENetAddress cliaddress;
        enet_address_set_host_ip(&cliaddress, "127.0.0.1");
        cliaddress.port = hostaddress.port;
        client = newClient(cliaddress);
        client->setEventListener(new RDMNetListener(m_playerSceneManager));

        m_viewport = new mtx::Viewport(640, 480);
        m_viewport->setClearColor(glm::vec4(0.4,0.4,0.4,1));
        m_viewport->setNearDistance(1.f);
        m_viewport->setFarDistance(65535.f);
        
        m_window = newWindow(m_viewport, mtx::HWAPI::HWWT_NORMAL_RESIZABLE);
        m_window->setTitle("RDM alpha");
        // m_window->getHwWindow()->setGrab(true);
    
        m_uiViewport = new mtx::Viewport(640, 480);
        m_uiViewport->setClearColor(glm::vec4(0));

        m_physworld = new mtx::PhysicsWorld();
        m_sceneManager->getRootNode()->addComponent(m_physworld);

        m_eventListener = new RDMListener(this);
        App::getHWAPI()->addListener(m_eventListener);

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_sceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        mtx::SceneNode* healthNode = new mtx::SceneNode();
        healthNode->setParent(m_uiSceneManager->getRootNode());
        mtx::ModelComponent* healthModel = new mtx::ModelComponent("models/cross.obj");
        mtx::MaterialComponent* healthMaterial = new mtx::MaterialComponent(mtx::Material::getMaterial("materials/diffuse.mmf"));
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
        mtx::SceneNode* logoNode = new mtx::SceneNode();
        m_guiShipStatus = new mtx::GUITextComponent();
        m_guiShipStatus->setText("RDM alpha");
        m_guiShipStatus->setFont(App::getHWAPI( )->loadCachedTexture("textures/font.png"));
        m_guiShipStatus->setOffset(glm::vec3(0.0, 480.0 - 16.0, 0.0));
        m_guiShipStatus->setCharacterSize(glm::ivec2(8,16));
        guiNode->addComponent(m_guiShipStatus);
        guiNode->setParent(m_uiSceneManager->getRootNode());

        mtx::GUIImageComponent* logoImage = new mtx::GUIImageComponent("textures/entropy_interactive_small.png");
        logoNode->addComponent(logoImage);
        logoNode->setParent(m_uiSceneManager->getRootNode());

        m_uiSceneManager->setSunDirection(glm::vec3(1,1,1));

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

        bsp = new mtx::BSPFile("rdm/rdm/maps/ffa_me2.bsp");
        bsp->addToPhysicsWorld(m_physworld);
        mtx::SceneNode* mapNode = new mtx::SceneNode();
        mtx::BSPComponent* mapComponent = new mtx::BSPComponent(bsp);
        mapNode->addComponent(mapComponent);
        mapNode->setParent(m_sceneManager->getRootNode());
        cameraps = glm::vec3(0,0,0);

        // TODO: RDMOcclusionTester (and thus the BSP testing solution) is too slow at the moment
        // getting rid of it saves about ~10ms in Dt
        //m_sceneManager->setOcclusionTester(new RDMOcclusionTester(bsp));

        m_cameraYaw = 0.f;
        m_cameraPitch = 0.f;
    }

    virtual void initGfx() {
        
    }

    virtual void tick() {
        mtx::SceneTransform& cameratf = cameraNode->getTransform();
        cameratf.setPosition(cameraps);

        m_cameraYaw += m_eventListener->xrel * getDeltaTime();
        m_cameraPitch += m_eventListener->yrel * getDeltaTime();

        float clock = 1.5f;
        m_cameraPitch = SDL_clamp(m_cameraPitch, -clock, clock);

        glm::quat qx = glm::angleAxis(m_cameraPitch, glm::vec3(1,0,0));
        glm::quat qy = glm::angleAxis(0.f, glm::vec3(0,1,0));
        glm::quat qz = glm::angleAxis(m_cameraYaw, glm::vec3(0,0,1));
        glm::quat qf = qx * qy * qz;
        glm::vec3 front = glm::vec3(0, 1, 0) * qf;
        glm::vec3 right = glm::vec3(1, 0, 0) * qf;

        float movespeed = 64.f;
        static bool debounce;
        if(m_eventListener->keysDown['w'])
            cameraps += movespeed * front * (float)getDeltaTime();
        if(m_eventListener->keysDown['s'])
            cameraps -= movespeed * front * (float)getDeltaTime();
        if(m_eventListener->keysDown['a'])
            cameraps -= movespeed * right * (float)getDeltaTime();
        if(m_eventListener->keysDown['d'])
            cameraps += movespeed * right * (float)getDeltaTime();

        m_eventListener->xrel = 0.f;
        m_eventListener->yrel = 0.f;

        cameratf.setWorldMatrix(glm::lookAt(
            cameraps,
            front + cameraps,
            glm::vec3(0,0,1)
        ));

        bsp->updatePosition(cameraps);

        char statustx[1024];
        snprintf(statustx, 1024, "RDM alpha\n"
                 "m_useVis: %s, vis cluster: %i\n"
                 "faces: %i/%i, leafs: %i/%i\n"
                 "vertices: %i, calls: %i\n"
                 "%f, %f, %f %f %f\n"
                 "Dt=%f T=%f\n"
                 "PDt=%f",
                 bsp->getUsingVis() ? "true" : "false",
                 bsp->getVisCluster(),
                 bsp->getFacesRendered(), bsp->getFacesTotal(),
                 bsp->getLeafsRendered(), bsp->getLeafsTotal(),
                 App::getHWAPI()->getDrawnVertices(), App::getHWAPI()->getDrawCalls(),
                 front.x, front.y, front.z, m_cameraYaw, m_cameraPitch,
                 getDeltaTime(), getExecutionTime(),
                 m_physworld->getDeltaTime());
        m_guiShipStatus->setText(statustx);
    } 
};


int main(int argc, char** argv)
{
    RDMApp app = RDMApp();
    
    app.initParameters(argc, argv);

    return app.main();
}