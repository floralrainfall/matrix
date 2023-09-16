#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>
#include <mwweb.hpp>
#include <mbsp.hpp>
#include <format>

class NRSimListener : public mtx::HWAPI::EventListener
{
public:
    bool keysDown[1024];
    mtx::App* app;

    NRSimListener(mtx::App* app)
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

class NRSimApp : public mtx::App
{
    mtx::SceneManager* m_sceneManager;
    mtx::Viewport* m_viewport;
    mtx::Window* m_window;
    mtx::Material* m_diffuse;
    mtx::SceneNode* cameraNode;
    NRSimListener* m_eventListener;
    mtx::GUITextComponent* m_guiShipStatus;

    float m_controlRods[8]; // 0.0 not inserted - 1.0 inserted
    float m_fuelRods[8]; // 0.0 - depleted - 1.0 full
    mtx::BSPFile* bsp;
public:
    virtual void init() {
        m_sceneManager = new mtx::SceneManager();
        addSceneManager(m_sceneManager);
        m_viewport = new mtx::Viewport(640, 480);
        m_viewport->setClearColor(glm::vec4(0.4,0.4,0.4,1));
        m_window = newWindow(m_viewport);
        m_window->setTitle("NRSim (RBMK 1000)");

        m_eventListener = new NRSimListener(this);
        App::getHWAPI()->addListener(m_eventListener);

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_sceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        m_diffuse = mtx::Material::getMaterial("materials/diffuse.mmf");

        mtx::SceneNode* guiNode = new mtx::SceneNode();
        m_guiShipStatus = new mtx::GUITextComponent();
        m_guiShipStatus->setText("Welcome to NRSim\nSelect moderator rod with < or >");
        m_guiShipStatus->setFont(App::getHWAPI()->loadCachedTexture("textures/font.png"));
        m_guiShipStatus->setOffset(glm::vec3(0.0, 480.0 - 16.0, 0.0));
        m_guiShipStatus->setCharacterSize(glm::ivec2(8,16));
        guiNode->addComponent(m_guiShipStatus);
        guiNode->setParent(m_sceneManager->getRootNode());

        mtx::SceneNode* reactorNode = new mtx::SceneNode();
        mtx::ModelComponent* reactorModel = new mtx::ModelComponent("models/rbmk1000.obj");
        mtx::MaterialComponent* reactorMaterial = new mtx::MaterialComponent(m_diffuse);
        reactorNode->addComponent(reactorModel);
        reactorNode->addComponent(reactorMaterial);
        reactorNode->setParent(m_sceneManager->getRootNode());
    }

    virtual void tick() {
        mtx::SceneTransform cameratf = cameraNode->getTransform();
        cameratf.setWorldMatrix(glm::lookAt(
            glm::vec3(10,0,-3),
            glm::vec3(0,0,-3),
            glm::vec3(0,1,0)            
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
    NRSimApp app = NRSimApp();
    
    app.initParameters(argc, argv);

    return app.main();
}