#include <mscene.hpp>
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <mapp.hpp>

namespace mtx
{
    SceneManager::SceneManager()
    {
        m_rootNode = new SceneNode("Root");
        m_rootNode->m_manager = this;

        m_sunAmbient = glm::vec3(0.25,0.25,0.25);
        m_sunDiffuse = glm::vec3(0.50,0.50,0.50);
        m_sunAmbient = glm::vec3(1,1,1);
        m_sunDirection = glm::vec3(-0.5,-0.25,0.5);
    }

    void SceneManager::renderScene()
    {
        m_currentRelativeTransform = SceneTransform();

        HWRenderParameter rp;
        rp.name = "sun.ambient";
        rp.data.v3 = m_sunAmbient;
        rp.type = HWT_VECTOR3;
        App::getHWAPI()->pushParam(rp);
        rp.name = "sun.diffuse";
        rp.data.v3 = m_sunDiffuse;
        rp.type = HWT_VECTOR3;
        App::getHWAPI()->pushParam(rp);
        rp.name = "sun.specular";
        rp.data.v3 = m_sunSpecular;
        rp.type = HWT_VECTOR3;
        App::getHWAPI()->pushParam(rp);
        rp.name = "sun.direction";
        rp.data.v3 = m_sunDirection;
        rp.type = HWT_VECTOR3;
        App::getHWAPI()->pushParam(rp);

        if(m_rootNode)
            m_rootNode->renderNode();
    }

    void SceneManager::tickScene()
    {
        if(m_rootNode)
            m_rootNode->tickNode();
    }

    SceneTransform::SceneTransform()
    {
        m_position = glm::vec3(0);
        m_rotation = glm::quat();
        m_scale = glm::vec3(1);
        rebuildMatrix();
    }

    void SceneTransform::rebuildMatrix()
    {
        m_matrix = glm::identity<glm::mat4>();

        // M = S * R * T

        m_matrix = m_matrix + glm::scale(m_scale);
        m_matrix = m_matrix + glm::translate(m_position);
        m_matrix = m_matrix + glm::toMat4(m_rotation);
    }

    SceneTransform SceneTransform::operator +(SceneTransform o)
    {
        SceneTransform n;
        n.m_position = m_position + o.m_position;
        n.m_rotation = m_rotation + o.m_rotation;
        return n;
    }

    SceneNode::SceneNode(std::string name)
    {
        m_name = name;
        m_parent = NULL;
        m_manager = NULL;
    }

    SceneNode::~SceneNode()
    {
        for(auto child : m_children)
            child->setParent(NULL);
        for(auto component : m_components)
            delete component;
    }

    void SceneNode::addComponent(SceneComponent* component)
    {
        m_components.push_back(component);
        component->setAttachedNode(this);
    }

    SceneComponent* SceneNode::getComponent(const char* type)
    {
        for(auto component : m_components)
        {
            if(component->className() == type)
                return component;
        }
        return NULL;
    }

    void SceneNode::addNewChild(SceneNode* node)
    {
        if(node->getParent() != this)
            return;
        m_children.push_back(node);
        node->m_manager = m_manager;
    }

    void SceneNode::removeChild(SceneNode* node)
    {
        auto it = std::find(m_children.begin(), m_children.end(), node);
        if(it != m_children.end())
            m_children.erase(it);
        node->m_manager = 0;
    }

    void SceneNode::setParent(SceneNode* parent)
    {
        if(parent == this)
            return;
        if(m_parent)
            m_parent->removeChild(this);
        m_parent = parent;
        m_parent->addNewChild(this);
    }

    void SceneNode::renderNode()
    {
        if(!m_manager)
            return;
        SceneTransform ot = m_manager->m_currentRelativeTransform;
        SceneTransform t = m_manager->m_currentRelativeTransform + m_transform;
        m_manager->m_currentRelativeTransform = t;
        for(auto component : m_components)
            component->renderComponent();
        for(auto child : m_children)
            child->renderNode();
        m_manager->m_currentRelativeTransform = ot;
    }

    void SceneNode::tickNode()
    {
        for(auto component : m_components)
            component->tick();
        for(auto child : m_children)
            child->tickNode();
    }
}