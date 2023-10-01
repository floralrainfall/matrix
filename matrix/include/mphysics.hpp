#pragma once
#include <mscene.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace mtx
{
    class BulletHelpers
    {
    public:
        static btVector3 v3ToBullet(glm::vec3 vec3)
        {
            return btVector3(vec3.x, vec3.y, vec3.z);
        }

        static btVector4 v4ToBullet(glm::vec4 vec4)
        {
            return btVector4(vec4.x, vec4.y, vec4.z, vec4.w);
        }

        static btQuaternion qtToBullet(glm::quat qt)
        {
            return btQuaternion(qt.x, qt.y, qt.z, qt.w);
        }

        static glm::vec3 v3FromBullet(btVector3 vec3)
        {
            return glm::vec3(vec3.x(), vec3.y(), vec3.z());
        }

        static glm::vec4 v4FromBullet(btVector4 vec4)
        {
            return glm::vec4(vec4.x(), vec4.y(), vec4.z(), vec4.w());
        }

        static glm::quat qtFromBullet(btQuaternion qt)
        {
            return glm::quat(qt.w(), qt.x(), qt.y(), qt.z());
        }

        static glm::mat4 m4FromBullet(btTransform tf)
        {
            glm::mat4 m;
            tf.getOpenGLMatrix((btScalar*)&m);
            return m;
        }
    };

    // put this in the root scene node
    class PhysicsWorld : public SceneComponent
    {
        btDefaultCollisionConfiguration* m_config;
        btDiscreteDynamicsWorld* m_world;
        btSequentialImpulseConstraintSolver* m_solver;
        btBroadphaseInterface* m_broadphase;
        btCollisionDispatcher* m_dispatcher;

        double m_deltaTime;
    public:
        PhysicsWorld();

        void setGravity(btVector3 v = {0, 0, -10});
        void addStaticObject(btCollisionObject* shape);
        void addRigidBody(btRigidBody* body);
        double getDeltaTime() { return m_deltaTime; }

        virtual void tick();
    };

    class RigidBody : public SceneComponent
    {
        btMotionState* m_motionState;
        btRigidBody* m_body;
    public:
        RigidBody(btCollisionShape* shape, btScalar mass);
        btTransform getTransform() { return m_body->getWorldTransform(); };
        void setTransform(const btTransform tf) { m_body->setWorldTransform(tf); }
        void addToWorld(PhysicsWorld* world);

        virtual void tick();
    };
}