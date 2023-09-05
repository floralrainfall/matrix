#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace mtx
{
    class SceneNode;

    class SceneTransform
    {
    private:
        glm::vec3 m_position;
        glm::quat m_rotation;
        glm::vec3 m_scale;
        glm::mat4 m_matrix;
    public:
        SceneTransform();
        void setPosition(glm::vec3 pos) { m_position = pos; rebuildMatrix(); }
        glm::vec3 getPosition() { return m_position; }
        void setRotation(glm::quat rot) { m_rotation = rot; rebuildMatrix(); }
        glm::quat getRotation() { return m_rotation; }
        void setScale(glm::vec3 scale)  { m_scale = scale; rebuildMatrix(); }
        glm::vec3 getScale() { return m_scale; }
        void rebuildMatrix();
        void setWorldMatrix(glm::mat4 mat) { m_matrix = mat; }
        glm::mat4 getWorldMatrix() { return m_matrix; }

        SceneTransform operator +(SceneTransform o);
    };
    
    class SceneManager
    {
        friend class SceneNode;
        SceneNode* m_rootNode;

        SceneTransform m_currentRelativeTransform;

        glm::vec3 m_sunAmbient;
        glm::vec3 m_sunDiffuse;
        glm::vec3 m_sunSpecular;
        glm::vec3 m_sunDirection;
    public:
        SceneManager();

        void renderScene();
        void tickScene();
        SceneNode* getRootNode() { return m_rootNode; }
    };

    class SceneComponent
    {
    protected:
        SceneNode* m_node;
    public:
        virtual void tick() {};
        virtual void renderComponent() {};
        virtual std::string className() { return "SceneComponent"; }

        void setAttachedNode(SceneNode* node) { m_node = node; };
    };

    class SceneComponentFactory
    {
    public:
        SceneComponent* buildComponent();
        void registerComponent();
    };

    class SceneNode
    {
    private:
        friend class SceneManager;
        std::vector<SceneNode*> m_children;
        std::vector<SceneComponent*> m_components;
        SceneNode* m_parent;
        SceneManager* m_manager;
        SceneTransform m_transform;
        std::string m_name;

        void addNewChild(SceneNode* node);
        void removeChild(SceneNode* node);
    public:
        SceneNode(std::string name = "Node");
        ~SceneNode();

        void setParent(SceneNode* parent);
        SceneNode* getParent() { return m_parent; }

        void addComponent(SceneComponent* component);
        SceneComponent* getComponent(const char* type);

        void setTransform(SceneTransform t) { m_transform = t; }
        SceneTransform& getTransform() { return m_transform; }

        SceneManager* getScene() { return m_manager; }

        void tickNode();
        void renderNode();
    };
};