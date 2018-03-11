/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#ifndef PEGASUS_COLLISION_HPP
#define PEGASUS_COLLISION_HPP

#include <pegasus/Scene.hpp>
#include <Arion/SimpleShapeIntersectionDetector.hpp>
#include <unordered_set>

namespace pegasus
{
namespace collision
{

/**
 * @brief Stores contact information
 */
struct Contact
{
    struct Manifold;

    Contact(mechanics::Body& aBody, mechanics::Body& bBody, Manifold manifold, double restitution);

    /**
     * @brief Stores contact manifold data
     */
    struct Manifold
    {
        //!Contact point in the world space on the a body
        glm::dvec3 aWorldContactPoint;

        //!Contact point in the world space on the b body
        glm::dvec3 bWorldContactPoint;

        //!Contact point in the local space on the a body
        glm::dvec3 aModelContactPoint;

        //!Contact point in the local space on the b body
        glm::dvec3 bModelContactPoint;

        //!Contact normal from the perspective of the b body
        glm::dvec3 normal;

        //!Penetration depth of two bodies
        double penetration;
    };

    mechanics::Body* aBody;
    mechanics::Body* bBody;
    Manifold manifold;

    //!Factor that's responsible for calculating the amount of energy lost to the deformation
    double restitution;
};

/**
 * @brief Detects collisions
 */
class Detector
{
public:
    /**
     * @brief Construct detector initialized with a given asset manager
     * @param assetManager
     */
    Detector(scene::AssetManager& assetManager);

    /**
     * @brief Detects and returns contacts
     * @return contacts
     */
    std::vector<std::vector<Contact>> Detect();

    //!Default restitution factor for collision manifests
    static double constexpr restitutionCoefficient = 0.75;

private:
    struct ObjectHasher
    {
        template < typename ObjectTypeA, typename ObjectTypeB = ObjectTypeA >
        size_t operator()(std::pair<ObjectTypeA*, ObjectTypeB*> data) const
        {
            return std::hash<ObjectTypeA*>()(data.first) ^ std::hash<ObjectTypeB*>()(data.second);
        }
    };

    scene::AssetManager* m_assetManager = nullptr;
    static arion::SimpleShapeIntersectionDetector s_simpleShapeDetector;

    /**
     * @brief Checks if two shapes are intersecting
     * @param aShape first shape
     * @param bShape second shape
     * @return @c true if shapes are intersecting, @c false otherwise
     */
    static bool Intersect(
        arion::SimpleShape const* aShape, arion::SimpleShape const* bShape
    );

    /**
     * @brief Calculates contact manifold for two intersecting shapes
     * @param aShape first shape
     * @param bShape second shapes
     * @return contact manifold
     */
    static Contact::Manifold CalculateContactManifold(
        arion::SimpleShape const* aShape, arion::SimpleShape const* bShape
    );

    /**
     * @brief Detects collisions in the given set of rigid bodies
     * @tparam Object rigid body type
     * @tparam Shape shape type
     * @return contacts
     */
    template < typename Object, typename Shape >
    std::vector<Contact> Detect()
    {
        std::vector<Contact> contacts;
        std::unordered_set<std::pair<Shape*, Shape*>, ObjectHasher> registeredContacts;
        std::vector<scene::Asset<scene::RigidBody>>& objects = m_assetManager->GetObjects<Object, Shape>();

        for (scene::Asset<scene::RigidBody> aObject : objects)
        {
            for (scene::Asset<scene::RigidBody> bObject : objects)
            {
                if (aObject.id == 0 || bObject.id == 0)
                {
                    continue;
                }

                mechanics::Body& aBody = m_assetManager->GetAsset(m_assetManager->GetBodies(), aObject.data.body);
                mechanics::Body& bBody = m_assetManager->GetAsset(m_assetManager->GetBodies(), bObject.data.body);
                if (aBody.material.HasInfiniteMass() && bBody.material.HasInfiniteMass())
                {
                    continue;
                }

                Shape* aShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<Shape>(), aObject.data.shape);
                Shape* bShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<Shape>(), bObject.data.shape);
                if (aShape == bShape)
                {
                    continue;
                }

                std::pair<Shape*, Shape*> const key = std::make_pair(std::min(aShape, bShape), std::max(aShape, bShape));
                if (Intersect(aShape, bShape)
                    && registeredContacts.find(key) == registeredContacts.end())
                {
                    contacts.emplace_back(
                        std::ref(aBody), std::ref(bBody), CalculateContactManifold(aShape, bShape), restitutionCoefficient
                    );
                    registeredContacts.insert(key);
                }
            }
        }

        return contacts;
    }

    /**
     * @brief Calculates contacts between two sets of rigid bodies
     * @tparam ObjectA rigid body type
     * @tparam ShapeA shape type
     * @tparam ObjectB rigid body type
     * @tparam ShapeB shape type
     * @return contacts
     */
    template < typename ObjectA, typename ShapeA, typename ObjectB, typename ShapeB >
    std::vector<Contact> Detect()
    {
        std::vector<Contact> contacts;
        std::unordered_set<std::pair<void*, void*>, ObjectHasher> registeredContacts;
        std::vector<scene::Asset<scene::RigidBody>>& aObjects = m_assetManager->GetObjects<ObjectA, ShapeA>();
        std::vector<scene::Asset<scene::RigidBody>>& bObjects = m_assetManager->GetObjects<ObjectB, ShapeB>();

        for (scene::Asset<scene::RigidBody> aObject : aObjects)
        {
            for (scene::Asset<scene::RigidBody> bObject : bObjects)
            {
                if (aObject.id == 0 || bObject.id == 0)
                {
                    continue;
                }

                mechanics::Body& aBody = m_assetManager->GetAsset(m_assetManager->GetBodies(), aObject.data.body);
                mechanics::Body& bBody = m_assetManager->GetAsset(m_assetManager->GetBodies(), bObject.data.body);
                if (aBody.material.HasInfiniteMass() && bBody.material.HasInfiniteMass())
                {
                    continue;
                }

                ShapeA* aShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<ShapeA>(), aObject.data.shape);
                ShapeB* bShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<ShapeB>(), bObject.data.shape);
                std::pair<void*, void*> const key = std::make_pair(
                    std::min(static_cast<void*>(aShape), static_cast<void*>(bShape)),
                    std::max(static_cast<void*>(aShape), static_cast<void*>(bShape))
                );

                if (Intersect(aShape, bShape)
                    && registeredContacts.find(key) == registeredContacts.end())
                {
                    contacts.emplace_back(
                        std::ref(aBody), std::ref(bBody), CalculateContactManifold(aShape, bShape), restitutionCoefficient
                    );
                    registeredContacts.insert(key);
                }
            }
        }

        return contacts;
    }
};

class Resolver
{
public:
    uint32_t iterationsUsed;
    uint32_t iterations = 10000;

    /**
     * @brief Resolves collisions
     * @param contacts contacts information
     * @param duration delta time of the frame
     */
    void Resolve(std::vector<std::vector<Contact>>& contacts, double duration);

private:
    /**
     * @brief Updates velocities of the bodies in the contact
     * @param contact contact information
     * @param duration delta time of the frame
     */
    static void ResolveVelocity(Contact contact, double duration);

    /**
     * @brief Updated positions of the bodies in the contact
     * @param contact contact information
     */
    static void ResolveInterpenetration(Contact contact);

    /**
     * @brief Resolves contact
     * @param contact contact information
     * @param duration delta time of the frame
     */
    static void Resolve(Contact contact, double duration);
};

} // namespace collision
} // namespace pegasus

#endif //PEGASUS_COLLISION_HPP
