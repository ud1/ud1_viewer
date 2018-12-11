#ifndef MYUTILS3D_HPP
#define MYUTILS3D_HPP

#include "myutils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/transform.hpp>

typedef glm::vec3 V3;
typedef glm::quat Q;
typedef glm::mat4 M4;

#ifdef ENABLE_LOGGING
inline std::ostream &operator << ( std::ostream &str, const V3 &p )
{
    str << "(" << p.x << "," << p.y << "," << p.z << ")";
    return str;
}
#endif

#endif // MYUTILS3D_HPP
