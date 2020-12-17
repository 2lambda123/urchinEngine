#include <algorithm>

#include "body/BodyManager.h"

namespace urchin {

    BodyManager::BodyManager() :
        lastUpdatedBody(nullptr) {

    }

    BodyManager::~BodyManager() {
        for (auto& body : bodies) {
            delete body;
        }

        for (auto& body : newBodies) {
            delete body;
        }
    }

    void BodyManager::addBody(AbstractBody* body) {
        std::lock_guard<std::mutex> lock(bodiesMutex);

        newBodies.push_back(body);
    }

    void BodyManager::removeBody(AbstractBody* body) {
        std::lock_guard<std::mutex> lock(bodiesMutex);

        bodiesToDelete.push_back(body);
    }

    AbstractBody* BodyManager::getLastUpdatedBody() const {
        return lastUpdatedBody;
    }

    const std::vector<AbstractBody*>& BodyManager::getBodies() const {
        return bodies;
    }

    /**
     * Refresh bodies list
     */
    void BodyManager::refreshBodies() {
        ScopeProfiler sp(Profiler::physics(), "setupWorkBodies");
        std::lock_guard<std::mutex> lock(bodiesMutex);

        //add new bodies
        bodies.insert(bodies.end(), newBodies.begin(), newBodies.end());
        for(const auto newBody : newBodies) {
            lastUpdatedBody = newBody;
            notifyObservers(this, ADD_BODY);
        }
        newBodies.clear();

        //delete bodies
        for(const auto bodyToDelete: bodiesToDelete) {
            auto itFind = std::find(bodies.begin(), bodies.end(), bodyToDelete);
            if(itFind != bodies.end()) {
                bodies.erase(itFind);

                lastUpdatedBody = bodyToDelete;
                notifyObservers(this, REMOVE_BODY);

                delete bodyToDelete;
            }
        }
        bodiesToDelete.clear();

        //TODO remove
//        if (body->needFullRefresh()) {
//            deleteWorkBody(body);
//            createNewWorkBody(body);
//            ++it;
//        }
    }

}
