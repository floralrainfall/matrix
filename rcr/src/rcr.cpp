#include <mapp.hpp>
#include <mcfg.hpp>
#include <mdev.hpp>
#include <mmaterial.hpp>
#include <mmodel.hpp>
#include <mgui.hpp>

class RCRListener : public mtx::Window::EventListener
{
public:
    bool keysDown[1024];

    virtual void onCloseWindow(mtx::Window* window)
    {
        window->setShouldClose(true);
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

class RCRApp : public mtx::App
{
    mtx::SceneManager* m_sceneManager;
    mtx::Viewport* m_viewport;
    mtx::Window* m_window;
    mtx::Material* m_diffuse;
    mtx::SceneNode* cameraNode;
    mtx::SceneNode* playerNode;
    RCRListener* m_eventListener;

public:
    virtual void init() {
        m_sceneManager = new mtx::SceneManager();
        m_viewport = new mtx::Viewport(640, 480);
        m_window = newWindow(m_viewport);
        //m_window->setTitle("River City Rampage");

        m_eventListener = new RCRListener();
        m_window->addListener(m_eventListener);

        cameraNode = new mtx::SceneNode();
        cameraNode->setParent(m_sceneManager->getRootNode());
        m_viewport->setCameraNode(cameraNode);

        m_diffuse = mtx::Material::getMaterial("materials/diffuseTextured.mmf");

        mtx::SceneNode* guiNode = new mtx::SceneNode();
        mtx::GUIImageComponent* guiImage = new mtx::GUIImageComponent("textures/peter.png");
        guiNode->addComponent(guiImage);
        guiNode->setParent(m_sceneManager->getRootNode());

        for(int i = 0; i < 50; i++)
        {
            mtx::SceneNode* testNode = new mtx::SceneNode();
            testNode->setParent(m_sceneManager->getRootNode());
            testNode->getTransform().setPosition(glm::vec3(0.f,0.f,-i*80));
            mtx::ModelComponent* testModel = new mtx::ModelComponent("models/sidewalk0.glb");
            testNode->addComponent(testModel);
            mtx::MaterialComponent* testMaterialComp = new mtx::MaterialComponent(m_diffuse);
            testNode->addComponent(testMaterialComp);
        }

        playerNode = new mtx::SceneNode();
    }

    virtual void tick() {
        m_sceneManager->tickScene();

        mtx::SceneTransform& ptf = playerNode->getTransform();
        mtx::SceneTransform tf;
        tf.setPosition(glm::vec3(
            5.f,
            2.f,
            ptf.getPosition().z
        ));
        glm::mat4 lookAtMatrix = glm::lookAt(tf.getPosition(), glm::vec3(0,0,ptf.getPosition().z), glm::vec3(0,1,0));
        tf.setWorldMatrix(lookAtMatrix);
        
        float movespeed = 16.0;

        if(m_eventListener->keysDown['a'])
            ptf.setPosition(ptf.getPosition() + glm::vec3(0.0,0.0,movespeed*0.016));
        else if(m_eventListener->keysDown['d'])
            ptf.setPosition(ptf.getPosition() + glm::vec3(0.0,0.0,-movespeed*0.016));
        
        cameraNode->setTransform(tf);
    }
};

int main(int argc, char** argv)
{
    RCRApp app = RCRApp();
    
    app.initParameters(argc, argv);

    return app.main();
}