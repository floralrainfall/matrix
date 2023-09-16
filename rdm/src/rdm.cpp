#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>
#include <mwweb.hpp>
#include <mbsp.hpp>
#include <format>

class RDMListener : public mtx::HWAPI::EventListener
{
public:
    bool keysDown[1024];
    mtx::App* app;

    RDMListener(mtx::App* app)
    {
        this->app = app;
    }

    virtual void onQuit()
    {
        app->setAppRunning(false);
    }

    virtual void onKeyDown(int key)
    {
        keysDown[key] = true;
    }

    virtual void onKeyUp(int key)
    {
        keysDown[key] = false;
    }
};

class RDMApp : public mtx::App
{
    mtx::SceneManager* m_sceneManager;
    mtx::Viewport* m_viewport;
    mtx::Window* m_window;
    mtx::Material* m_diffuse;
    mtx::SceneNode* cameraNode;
    RDMListener* m_eventListener;
    mtx::GUITextComponent* m_guiShipStatus;
    mtx::BSPFile* bsp;
public:
    virtual void init() {
        m_sceneManager = new mtx::SceneManager();
        addSceneManager(m_sceneManager);
        m_viewport = new mtx::Viewport(640, 480);
        m_viewport->setClearColor(glm::vec4(0.4,0.4,0.4,1));
        m_viewport->setNearDistance(1.f);
        m_viewport->setFarDistance(1000.f);
        m_window = newWindow(m_viewport);
        m_window->setTitle("RDM alpha");

        m_eventListener = new RDMListener(this);
        App::getHWAPI()->addListener(m_eventListener);

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_sceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        m_diffuse = mtx::Material::getMaterial("materials/bsp.mmf");

        mtx::SceneNode* guiNode = new mtx::SceneNode();
        m_guiShipStatus = new mtx::GUITextComponent();
        m_guiShipStatus->setText("RDM alpha");
        m_guiShipStatus->setFont(App::getHWAPI()->loadCachedTexture("textures/font.png"));
        m_guiShipStatus->setOffset(glm::vec3(0.0, 480.0 - 16.0, 0.0));
        m_guiShipStatus->setCharacterSize(glm::ivec2(8,16));
        guiNode->addComponent(m_guiShipStatus);
        guiNode->setParent(m_sceneManager->getRootNode());

        mtx::BSPFile* bsp = new mtx::BSPFile("rdm/rdm/maps/test.bsp");
        mtx::SceneNode* mapNode = new mtx::SceneNode();
        mtx::BSPComponent* mapComponent = new mtx::BSPComponent(bsp);
        mtx::MaterialComponent* reactorMaterial = new mtx::MaterialComponent(m_diffuse);
        mapNode->addComponent(mapComponent);
        mapNode->addComponent(reactorMaterial);
        mapNode->setParent(m_sceneManager->getRootNode());
    }

    virtual void tick() {
        mtx::SceneTransform cameratf = cameraNode->getTransform();
        cameratf.setWorldMatrix(glm::lookAt(
            glm::vec3(sin(getExecutionTime()/2.0)*255.0,cos(getExecutionTime()/2.0)*255.0,128),
            glm::vec3(-64,0,128),
            glm::vec3(0,0,1)            
        ));
        cameraNode->setTransform(cameratf);

        static bool debounce;
        if(m_eventListener->keysDown['<'])
        {
            
        }
    } 
};

int main(int argc, char** argv)
{
    RDMApp app = RDMApp();
    
    app.initParameters(argc, argv);

    return app.main();
}