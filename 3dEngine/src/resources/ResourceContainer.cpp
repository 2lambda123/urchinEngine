#include <UrchinCommon.h>

#include <resources/ResourceContainer.h>

namespace urchin {

    ResourceContainer::ResourceContainer() :
            ThreadSafeSingleton<ResourceContainer>() {

    }

    ResourceContainer::~ResourceContainer() {
        cleanResources();
        for (auto& resource : resources) {
            Logger::instance().logError("Resources not released: " + resource.second->getName());
        }
    }

    void ResourceContainer::addResource(const std::shared_ptr<Resource>& resource) {
        std::lock_guard<std::mutex> lock(mutex);
        resources.emplace(resource->getId(), resource);
    }

    void ResourceContainer::cleanResources() {
        std::lock_guard<std::mutex> lock(mutex);

        for (auto it = resources.begin(); it != resources.end();) {
            if (it->second.use_count() <= 1) { //resource not used anymore
                it = resources.erase(it);
            } else {
                ++it;
            }
        }
    }

}
