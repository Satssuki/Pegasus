/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#ifndef PEGASUS_COLLISION_HPP
#define PEGASUS_COLLISION_HPP

#include <pegasus/Scene.hpp>
#include <pegasus/Geometry.hpp>

namespace pegasus
{
namespace collision
{

struct Contact
{
    struct Manifold;

    Contact(mechanics::Body& aBody, mechanics::Body& bBody, Manifold manifold, double restitution);

    struct Manifold
    {
        glm::dvec3 normal;
        double penetration;
    };

    mechanics::Body* aBody;
    mechanics::Body* bBody;
    Manifold manifold;
    double restitution;
};

class Detector
{
public:
    Detector(scene::AssetManager& assetManager);

    std::vector<std::vector<Contact>> Detect();

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
    static geometry::SimpleShapeIntersectionDetector s_simpleShapeDetector;

    static bool Intersect(
        geometry::SimpleShape const* aShape, geometry::SimpleShape const* bShape
    );

    static Contact::Manifold CalculateContactManifold(
        geometry::SimpleShape const* aShape, geometry::SimpleShape const* bShape
    );

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
                Shape* aShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<Shape>(), aObject.data.shape);
                Shape* bShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<Shape>(), bObject.data.shape);
                std::pair<Shape*, Shape*> const key = std::make_pair(aShape, bShape);

                if (aShape != bShape
                    && Intersect(aShape, bShape)
                    && registeredContacts.find(key) == registeredContacts.end())
                {
                    contacts.emplace_back(
                        std::ref(m_assetManager->GetAsset(m_assetManager->GetBodies(), aObject.data.body)),
                        std::ref(m_assetManager->GetAsset(m_assetManager->GetBodies(), bObject.data.body)),
                        CalculateContactManifold(aShape, bShape),
                        0.75
                    );
                    registeredContacts.insert(key);
                }
            }
        }

        return contacts;
    }

    template < typename ObjectA, typename ShapeA, typename ObjectB, typename ShapeB >
    std::vector<Contact> Detect()
    {
        std::vector<Contact> contacts;
        std::unordered_set<std::pair<ShapeA*, ShapeB*>, ObjectHasher> registeredContacts;
        std::vector<scene::Asset<scene::RigidBody>>& aObjects = m_assetManager->GetObjects<ObjectA, ShapeA>();
        std::vector<scene::Asset<scene::RigidBody>>& bObjects = m_assetManager->GetObjects<ObjectB, ShapeB>();

        for (scene::Asset<scene::RigidBody> aObject : aObjects)
        {
            for (scene::Asset<scene::RigidBody> bObject : bObjects)
            {
                ShapeA* aShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<ShapeA>(), aObject.data.shape);
                ShapeB* bShape = &m_assetManager->GetAsset(m_assetManager->GetShapes<ShapeB>(), bObject.data.shape);
                std::pair<ShapeA*, ShapeB*> const key = std::make_pair(aShape, bShape);

                if (Intersect(aShape, bShape)
                    && registeredContacts.find(key) == registeredContacts.end())
                {
                    contacts.emplace_back(
                        std::ref(m_assetManager->GetAsset(m_assetManager->GetBodies(), aObject.data.body)),
                        std::ref(m_assetManager->GetAsset(m_assetManager->GetBodies(), bObject.data.body)),
                        CalculateContactManifold(aShape, bShape),
                        0.75
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

    void Resolve(std::vector<std::vector<Contact>>& contacts, double duration);

private:
    static double CalculateTotalSeparationSpeed(Contact contact);

    static double CalculatePureSeparationSpeed(Contact contact, double separationSpeed, double duration);

    static double CalculateSeparationSpeed(Contact contact, double duration);

    static glm::dvec3 CalculateTotalImpulse(Contact contact, double duration);

    static void ResolveVelocity(Contact contact, double duration);

    static void ResolveInterpenetration(Contact contact);

    static void Resolve(Contact contact, double duration);
};

} // namespace collision
} // namespace pegasus

#endif //PEGASUS_COLLISION_HPP