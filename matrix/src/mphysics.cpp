#include <mphysics.hpp>
#include <mapp.hpp>

namespace mtx
{
    PhysicsWorld::PhysicsWorld()
    {
        m_config = new btDefaultCollisionConfiguration();
        m_dispatcher = new btCollisionDispatcher(m_config);
        m_broadphase = new btDbvtBroadphase();
        m_solver = new btSequentialImpulseConstraintSolver();
        m_world = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_config);
        setGravity();
    }

    void PhysicsWorld::setGravity(btVector3 v)
    {
        m_world->setGravity(v);
    }

    void PhysicsWorld::tick()
    {
        App* app = m_node->getScene()->getApp();
        double start = m_node->getScene()->getApp()->getExecutionTime();
        m_world->stepSimulation(app->getDeltaTime(), 10);
        m_deltaTime = m_node->getScene()->getApp()->getExecutionTime() - start;
    }

    void PhysicsWorld::addStaticObject(btCollisionObject* object)
    {
        m_world->addCollisionObject(object);
    }

    void PhysicsWorld::addRigidBody(btRigidBody* body)
    {
        m_world->addRigidBody(body);
    };

    RigidBody::RigidBody(btCollisionShape* shape, btScalar mass)
    {
        m_motionState = new btDefaultMotionState();
        bool dynamic = (mass != 0.f);
        btVector3 inertia = btVector3();
        if(dynamic)
            shape->calculateLocalInertia(mass, inertia);
        m_body = new btRigidBody(mass, m_motionState, shape, inertia);
    }

    void RigidBody::addToWorld(PhysicsWorld* world)
    {
        mtx::SceneTransform& tf = m_node->getTransform();
        btTransform btf;
        btf.setIdentity();
        btf.setOrigin(BulletHelpers::v3ToBullet(tf.getPosition()));
        btf.setRotation(BulletHelpers::qtToBullet(tf.getRotation()));
        m_motionState->setWorldTransform(btf);
        world->addRigidBody(m_body);
    }

    void RigidBody::tick()
    {
        mtx::SceneTransform& tf =  m_node->getTransform();
        btTransform btf;
        m_motionState->getWorldTransform(btf);
        tf.setPosition(BulletHelpers::v3FromBullet(btf.getOrigin()));
        tf.setRotation(BulletHelpers::qtFromBullet(btf.getRotation()));        
    }
}