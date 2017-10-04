/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#ifndef PEGASUS_DEMO_HPP
#define PEGASUS_DEMO_HPP

#include <pegasus/ParticleWorld.hpp>
#include <demo/Render.hpp>

namespace pegasus
{
/**
 * @brief Represents a singleton instance of the Demo
 */
class Demo
{
public:
    struct Object;

    /**
     * @brief Returns a reference to the singleton Demo instance
     * @return Demo object reference
     */
    static Demo& GetInstance();

    /**
     * @brief Checks if the window is not closed and render is initialized
     * @return @c true if initialized, @c false otherwise
     */
    bool IsValid() const;

    /**
     * @brief Updates frame
     *
     * Runs physics calculations, renders new frame and swaps buffers
     */
    void RunFrame();

    /**
     * @brief Creates an object describing a line
     *
     * The object contains only render-related part and is not registered in the physics world
     *
     * @param[in] particle physical data
     * @param[in] start line start
     * @param[in] end line end
     * @return a newly created Object
     */
    Object& MakeLine(Particle particle, glm::vec3 start, glm::vec3 end);

    /**
     * @brief Creates an object describing a plane
     *
     * Particle::centerOfMass is used as the position of the object,
     * both physical and graphical
     *
     * @param[in] particle physical data
     * @param[in] normal normal of the vector
     * @return a newly created Object
     */
    Object& MakePlane(Particle particle, glm::dvec3 normal);

    /**
     * @brief Creates an object describing a sphere
     *
     * Particle::centerOfMass is used as the position of the object,
     * both physical and graphical
     *
     * @param[in] particle physical data
     * @param[in] radius radius of the sphere
     * @return a newly created Object
     */
    Object& MakeSphere(Particle particle, double radius);

    /**
     * @brief Creates an object describing a box
     *
     * Particle::centerOfMass is used as the position of the object,
     * both physical and graphical
     *
     * @param[in] particle physical data
     * @param[in] i orthogonal basis vector of the box base
     * @param[in] j orthogonal basis vector of the box base
     * @param[in] k orthogonal basis vector of the box base
     * @return a newly created Object
     */
    Object& MakeBox(Particle particle, glm::vec3 i, glm::vec3 j, glm::vec3 k);

    /**
     * @brief Removes object from the demo
     * @param[in] object reference to the object to be deleted
     */
    void Remove(Object& object);

    //! Maximum number of particles in the demo
    uint32_t const maxParticles = 200;

    /**
     * @brief Represents an instance of the render and physical objects
     */
    struct Object
    {
        Object(RigidBody* body, std::unique_ptr<render::primitive::Primitive>&& renderShape);

        //! Physical body
        RigidBody* body;

        //! Render data
        std::unique_ptr<render::primitive::Primitive> shape;
    };

private:
    std::list<Object> m_objects;
    render::Renderer& m_pRenderer;
    ParticleWorld m_particleWorld;
    Particles m_particles;
    RigidBodies m_rigidBodies;
    ParticleContactGenerators m_particleContactGenerators;
    ParticleForceRegistry m_particleForceRegistry;
    ParticleGravity m_gravityForce;

    Demo();

    /**
     * @brief Runs physics calculations with given duration
     * @param duration duration of the physical frame
     */
    void ComputeFrame(double duration);

    /**
     * @brief Redraws the scene
     *
     * Rerenders the frame and updates current frame buffer
     */
    void RenderFrame() const;

    /**
     * @brief Makes rigid body
     *
     * Makes rigid body updates internal structures
     *
     * @param[in] particle physical data
     * @param[in] shape collision geometry shape
     * @return a newly created RigidBody
     */
    RigidBody& MakeRigidBody(Particle particle, std::unique_ptr<geometry::SimpleShape>&& shape);
};
} // namespace pegasus

#endif // PEGASUS_DEMO_HPP