#pragma once

#include "Common/BulletExtensions.h"

namespace CBP
{
    namespace Geometry
    {
        bool FindNiBound(
            Actor* a_actor,
            const BSFixedString& a_nodeName,
            Bullet::btBound &a_out);

    }
}