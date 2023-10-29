#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>
#include <format>

class ASGListener : public mtx::HWAPI::EventListener
{
public:
    bool keysDown[1024];
    mtx::App* app;

    ASGListener(mtx::App* app)
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

class ASGApp : public mtx::App
{
    mtx::SceneManager* m_sceneManager;
    mtx::Viewport* m_viewport;
    mtx::Window* m_window;
    mtx::Material* m_diffuse;
    mtx::SceneNode* cameraNode;
    mtx::SceneNode* playerNode;
    ASGListener* m_eventListener;
    mtx::GUITextComponent* m_guiShipStatus;
    
    glm::vec3 m_shipVelocity;
    glm::vec3 m_shipAngularVelocity;

public:
    ASGApp(int argc, char** argv) : App(argc, argv)
    {

    }
    
    virtual void init() {
        m_sceneManager = new mtx::SceneManager(this);
        m_viewport = new mtx::Viewport(640, 480);
        m_viewport->setClearColor(glm::vec4(0,0,0,0));
        m_window = newWindow(m_viewport);
        m_window->setTitle("Awesome Space Game");

        m_eventListener = new ASGListener(this);
        App::getHWAPI()->addListener(m_eventListener);

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_sceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        m_diffuse = mtx::Material::getMaterial("materials/diffuse.mmf");

        mtx::SceneNode* guiNode = new mtx::SceneNode();
        m_guiShipStatus = new mtx::GUITextComponent();
        m_guiShipStatus->setText("Im like hey whats up hello\nHi");
        m_guiShipStatus->setFont(App::getHWAPI()->loadCachedTexture("textures/font.png"));
        m_guiShipStatus->setOffset(glm::vec3(0.0, 480.0 - 16.0, 0.0));
        m_guiShipStatus->setCharacterSize(glm::ivec2(8,16));
        guiNode->addComponent(m_guiShipStatus);
        guiNode->setParent(m_sceneManager->getRootNode());

        playerNode = new mtx::SceneNode();            
        mtx::ModelComponent* testModel = new mtx::ModelComponent("models/spaceship.obj");
        playerNode->addComponent(testModel);
        mtx::MaterialComponent* testMaterialComp = new mtx::MaterialComponent(m_diffuse);
        playerNode->addComponent(testMaterialComp);
        playerNode->setParent(m_sceneManager->getRootNode());
    }

    virtual void tick() {
        m_sceneManager->tickScene();

        mtx::SceneTransform& ptf = playerNode->getTransform();
        mtx::SceneTransform tf;
        
        float movespeed = 16.0;

        ptf.setPosition(ptf.getPosition() + (m_shipVelocity * (float)getDeltaTime()));
        ptf.setRotation(ptf.getRotation() * glm::quat(m_shipAngularVelocity * (float)getDeltaTime()));
        playerNode->setTransform(ptf);

        glm::vec3 right = ptf.getRotation() * glm::vec3(1, 0, 0);
        glm::vec3 up = ptf.getRotation() * glm::vec3(0, 1, 0);
        glm::vec3 forward = ptf.getRotation() * glm::vec3(0, 0, 1);

        tf.setPosition(ptf.getPosition() * ((forward + up) * 3.f));        
        
        glm::mat4 lookAtMatrix = glm::lookAt(tf.getPosition(), ptf.getPosition(), up);
        tf.setWorldMatrix(lookAtMatrix);
        cameraNode->setTransform(tf);

        if(m_eventListener->keysDown['w'])
            m_shipVelocity += forward * (float)getDeltaTime();
        if(m_eventListener->keysDown['s'])
            m_shipVelocity -= forward * (float)getDeltaTime();
        if(m_eventListener->keysDown['a'])
            m_shipAngularVelocity += right * (float)getDeltaTime();
        if(m_eventListener->keysDown['d'])
            m_shipAngularVelocity -= right * (float)getDeltaTime();
        
        m_guiShipStatus->setText(std::format("Dt: {:1.10f}, FPS: {:10.2f}\nVelocity: {} {} {}\nAngular Velocity: {} {} {}\nHeading: {} {} {}\nPosition: {} {} {}",getDeltaTime(), 1.0/getDeltaTime(), m_shipVelocity.x,m_shipVelocity.y,m_shipVelocity.z,m_shipAngularVelocity.x,m_shipAngularVelocity.y,m_shipAngularVelocity.z,forward.x,forward.y,forward.z,ptf.getPosition().x,ptf.getPosition().y,ptf.getPosition().z));
        
    } 
};

int main(int argc, char** argv)
{
    ASGApp app = ASGApp(argc, argv);

    return app.main();
}
