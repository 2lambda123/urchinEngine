#include <algorithm>

#include "WaterManager.h"

namespace urchin
{

    void WaterManager::onCameraProjectionUpdate(const Camera *camera)
    {
        this->projectionMatrix = camera->getProjectionMatrix();
        for(const auto water : waters)
        {
            water->onCameraProjectionUpdate(projectionMatrix);
        }
    }

    void WaterManager::addWater(Water *water)
    {
        if(water!=nullptr)
        {
            waters.push_back(water);

            water->onCameraProjectionUpdate(projectionMatrix);
        }
    }

    void WaterManager::removeWater(Water *water)
    {
        if(water!=nullptr)
        {
            waters.erase(std::remove(waters.begin(), waters.end(), water), waters.end());
            delete water;
        }
    }

    void WaterManager::display(const Camera *camera, float invFrameRate) const
    {
        for(const auto water : waters)
        {
            water->display(camera, invFrameRate);
        }
    }
}
