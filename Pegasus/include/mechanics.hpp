/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#ifndef PEGASUS_MECHANICS_HPP
#define PEGASUS_MECHANICS_HPP

#include "Pegasus/include/geometry.hpp"
#include "Pegasus/include/particle.hpp"

#include <memory>

namespace pegasus {

class RigidBody {
public:
    Particle & p;
    std::unique_ptr<geometry::SimpleShape> const s;

public:
    RigidBody(Particle & p, std::unique_ptr<geometry::SimpleShape> && s);
};

} // namespace pegasus
#endif // PEGASUS_MECHANICS_HPP
