#pragma once

#include "glm/glm.hpp"

namespace lb {

struct ProjInfo
{
    float nearZ;
    float farZ;
    float fov;
    float aspectRatio;
};

}
