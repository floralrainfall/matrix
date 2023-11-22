#include <mapp.hpp>
#include <player.hpp>
#include <game.hpp>
#include <input.hpp>
#include <mmodel.hpp>
#include <mmaterial.hpp>

class AwesopsApp : public mtx::App
{
    mtx::Window* m_window;
    mtx::Viewport* m_viewport;
    mtx::SceneManager* m_scene;

    awesops::LocalPlayer* m_localPlayer;
public:
    AwesopsApp(int argc, char** argv) : mtx::App(argc, argv)
    {
	
    }

    virtual void init()
    {	
	m_viewport = new mtx::Viewport(640, 480);
	m_window = newWindow(m_viewport, mtx::HWAPI::HWWT_NORMAL);
	m_scene = new mtx::SceneManager(this);

	m_localPlayer = 0;
	
	awesops::globalGame.init(m_scene);
    }

    virtual void initGfx()
    {
	mtx::SceneNode* camera = new mtx::SceneNode();
	camera->setParent(m_scene->getRootNode());

	m_viewport->setCameraNode(camera);

	awesops::globalGame.getPlayerManager()->loadResources();
	
	m_localPlayer = new awesops::LocalPlayer();
	m_localPlayer->setCameraNode(camera);

	mtx::SceneNode* playerNode = new mtx::SceneNode();
	playerNode->setParent(m_scene->getRootNode());

	mtx::ModelComponent* playerModel =
	    new mtx::ModelComponent("models/cross.obj");
	mtx::MaterialComponent* playerMaterial =
	    new mtx::MaterialComponent(
		mtx::Material::getMaterial("materials/diffuse.mmf"));
	playerNode->addComponent(playerModel);
	playerNode->addComponent(playerMaterial);
	
	m_localPlayer->setSceneNode(playerNode);
	
	awesops::globalGame.getPlayerManager()->addPlayer(m_localPlayer);
    }

    virtual void tick()
    {
	awesops::globalGame.tick();

	if(awesops::globalGame.getInputManager()->getShouldQuit())
	    setAppRunning(false);
    }
};

int main(int argc, char** argv)
{
    AwesopsApp app = AwesopsApp(argc, argv);

    return app.main();
}
