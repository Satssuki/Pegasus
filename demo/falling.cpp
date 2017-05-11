#include "demo/app.hpp"
#include "demo/ogl_headers.hpp"
#include "demo/timing.hpp"
#include "Pegasus/include/geometry.hpp"
#include "Pegasus/include/mechanics.hpp"
#include "Pegasus/include/particlecontacts.hpp"
#include "Pegasus/include/particleforcegenerator.hpp"
#include "Pegasus/include/particleworld.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>

#define PLANE_COUNT 1
#define BOX_COUNT 1
#define SPHERE_COUNT 1
#define TOTAL_COUNT BOX_COUNT+SPHERE_COUNT+PLANE_COUNT
#define RADIUS 1

class FallingDemo : public Application {
public:
    FallingDemo();
    virtual ~FallingDemo() {}

    const char* getTitle() override;
    void display() override;
    void update() override;
    void key(unsigned char key) override;
    void mouseDrag(int x, int y) override;

private:
    pegasus::RigidBodies rigidBodies;
    std::vector<pegasus::Particle::Ptr> particles;
    std::vector<pegasus::ParticleContactGenerator::Ptr> contactGenerators;
    pegasus::ParticleForceRegistry::Ptr forceRegistry;
    pegasus::ParticleWorld world;

    float xAxis;
    float yAxis;
};

// Method definitions
FallingDemo::FallingDemo()
    : forceRegistry(std::make_shared<pegasus::ParticleForceRegistry>())
    , world(1 + TOTAL_COUNT + 1, 1 + 1)
    , xAxis(0)
    , yAxis(0)
{
    //Create particles
    for (unsigned int i = 0; i < TOTAL_COUNT; ++i) {
        auto particle = std::make_shared<pegasus::Particle>();
        particle->setPosition(2.0f + RADIUS * i * 4, RADIUS, pegasus::real(0));

        particle->setVelocity(0, 0, 0);
        particle->setDamping(0.2f);
        particle->setMass(1.0f);
        particles.push_back(particle);

        if (i < SPHERE_COUNT) {
            rigidBodies.push_back(std::make_shared<pegasus::RigidBody>(
                particle,
                std::make_shared<pegasus::geometry::Sphere>(particle->getPosition(), RADIUS)));
        } else {
            rigidBodies.push_back(std::make_shared<pegasus::RigidBody>(
                particle,
                std::make_shared<pegasus::geometry::Box>(
                    particle->getPosition(),
                    pegasus::Vector3{ RADIUS, 0, 0 },
                    pegasus::Vector3{ 0, RADIUS, 0 },
                    pegasus::Vector3{ 0, 0, RADIUS })));
        }
    }

    auto particlePlane = std::make_shared<pegasus::Particle>();
    particlePlane->setPosition(pegasus::Vector3(1, 2, 0));
    particlePlane->setInverseMass(0);
    rigidBodies.push_back(std::make_shared<pegasus::RigidBody>(
        std::make_shared<pegasus::Particle>(),
        std::make_shared<pegasus::geometry::Plane>(particlePlane->getPosition(), pegasus::Vector3{0,1,0})
    ));

    //Create rigid bodies
    for (auto const& body : rigidBodies) {
        contactGenerators.push_back(
            std::make_shared<pegasus::ShapeContactGenerator>(body, rigidBodies, 0.0f));
    }

    //Init world
    world.setParticleContactGenerators(contactGenerators);
    world.setParticles(particles);
    world.setParticleForcesRegistry(forceRegistry);
}

void FallingDemo::display()
{
    // Clear the view port and set the camera direction
    auto const& pos = particles.front()->getPosition();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(pos.x, pos.y, 5, pos.x, pos.y, 0.0, 0.0, 1.0, 0.0);

    //Add bodies
    for (auto & body : rigidBodies)
    {
        auto const& p = body->p->getPosition();
        auto const& s = body->s->type;
        glPushMatrix();
        glColor3f(1.0f, 0.0f, 0.0f);
        glTranslatef(p.x, p.y, p.z);
        if (s == pegasus::geometry::SimpleShape::PLANE) {
            auto const p0 = static_cast<pegasus::geometry::Plane*>(body->s.get())->getCenterOfMass();
            auto const planeNormal = static_cast<pegasus::geometry::Plane*>(body->s.get())->getNormal();
            auto const posNormalProjection = planeNormal * (p0 * planeNormal);
            auto const p1 = p0 - posNormalProjection * 2;

            glBegin(GL_QUADS);
            glColor3f(0.18f, 0.31f, 0.31f);
            glVertex3f(p0.x, p0.y, p0.z + 25);
            glVertex3f(p1.x, p1.y, p1.z + 25);
            glVertex3f(p1.x, p1.y, p1.z - 25);
            glVertex3f(p0.x, p0.y, p0.z - 25);
            glEnd();
        }
        else if (s == pegasus::geometry::SimpleShape::SPHERE) {
            glutWireSphere(RADIUS, 20, 20);
        }
        else if (s == pegasus::geometry::SimpleShape::BOX) {
            glutWireCube(RADIUS * 2);
        }
        glPopMatrix();
    }
}

void FallingDemo::update()
{
    world.startFrame();

    auto duration = static_cast<float>(TimingData::get().lastFrameDuration * 0.001f);
    if (duration <= 0.0f)
        return;

    xAxis *= pow(0.1f, duration);
    yAxis *= pow(0.1f, duration);

    // Move the controlled blob
    particles.front()->addForce(pegasus::Vector3(xAxis, yAxis, 0) * 10.0f);

    world.runPhysics(0.001f); //(duration);

    for (auto const& body : rigidBodies) {
        body->s->setCenterOfMass(body->p->getPosition());
    }

    Application::update();
}

const char* FallingDemo::getTitle() { return "Pegasus Falling Demo"; }

void FallingDemo::mouseDrag(int x, int y)
{
}

void FallingDemo::key(unsigned char key)
{
    switch (key) {
    case 'w':
    case 'W':
        yAxis = 1.0;
        break;
    case 's':
    case 'S':
        yAxis = -1.0;
        break;
    case 'a':
    case 'A':
        xAxis = -1.0f;
        break;
    case 'd':
    case 'D':
        xAxis = 1.0f;
        break;
    default:
        break;
    }
}

Application* getApplication() { return new FallingDemo(); }