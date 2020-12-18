#include <stdexcept>
#include <utility>

#include "AbstractBody.h"
#include "collision/broadphase/PairContainer.h"

namespace urchin {

    //static
    uint_fast32_t AbstractBody::nextObjectId = 0;
    bool AbstractBody::bDisableAllBodies = false;

    AbstractBody::AbstractBody(std::string id, const PhysicsTransform& transform, std::shared_ptr<const CollisionShape3D> shape) :
            transform(transform),
            isManuallyMoved(false),
            ccdMotionThresholdFactor(ConfigService::instance()->getFloatValue("collisionShape.ccdMotionThresholdFactor")),
            bNeedFullRefresh(false),
            id(std::move(id)),
            shape(std::move(shape)),
            restitution(0.0f),
            friction(0.0f),
            rollingFriction(0.0f),
            ccdMotionThreshold(0.0f),
            bIsStatic(true),
            bIsActive(false),
            objectId(nextObjectId++) {
        initialize(0.2f, 0.5f, 0.0f);
    }

    AbstractBody::AbstractBody(const AbstractBody& abstractBody) :
            IslandElement(abstractBody),
            transform(abstractBody.getTransform()),
            isManuallyMoved(false),
            ccdMotionThresholdFactor(ConfigService::instance()->getFloatValue("collisionShape.ccdMotionThresholdFactor")),
            bNeedFullRefresh(false),
            id(abstractBody.getId()),
            shape(std::shared_ptr<const CollisionShape3D>(abstractBody.getShape()->clone())),
            restitution(0.0f),
            friction(0.0f),
            rollingFriction(0.0f),
            ccdMotionThreshold(0.0f),
            bIsStatic(true),
            bIsActive(false),
            objectId(nextObjectId++) {
        initialize(abstractBody.getRestitution(), abstractBody.getFriction(), abstractBody.getRollingFriction());

        setCcdMotionThreshold(abstractBody.getCcdMotionThreshold()); //override default value
    }

    void AbstractBody::setNeedFullRefresh(bool needFullRefresh) {
        this->bNeedFullRefresh.store(needFullRefresh, std::memory_order_relaxed);
    }

    bool AbstractBody::needFullRefresh() const {
        return bNeedFullRefresh.load(std::memory_order_relaxed);
    }

    void AbstractBody::initialize(float restitution, float friction, float rollingFriction) {
        //technical data
        bIsStatic.store(true, std::memory_order_relaxed);
        bIsActive.store(false, std::memory_order_relaxed);

        //shape check and data
        shape->checkInnerMarginQuality(id);
        ccdMotionThreshold = (shape->getMinDistanceToCenter() * 2.0f) * ccdMotionThresholdFactor;

        //body description data
        this->restitution = restitution;
        this->friction = friction;
        this->rollingFriction = rollingFriction;
    }

    void AbstractBody::setTransform(const PhysicsTransform& transform) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->transform = transform;

        //TODO handle this property if manual update
        //this->setNeedFullRefresh(true);
        //this->isManuallyMoved = true;
    }

    PhysicsTransform AbstractBody::getTransform() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return transform;
    }

    Point3<float> AbstractBody::getPosition() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return transform.getPosition();
    }

    Quaternion<float> AbstractBody::getOrientation() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return transform.getOrientation();
    }

    bool AbstractBody::isManuallyMovedAndResetFlag() {
        if (isManuallyMoved) {
            isManuallyMoved = false;
            return true;
        }
        return false;
    }

    const std::shared_ptr<const CollisionShape3D>& AbstractBody::getShape() const {
        return shape;
    }

    void AbstractBody::setId(const std::string& id) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->id = id;
    }

    const std::string& AbstractBody::getId() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return id;
    }

    void AbstractBody::setRestitution(float restitution) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->restitution = restitution;
    }

    /**
     * @return Restitution (0=stop, 1=100% elastic)
     */
    float AbstractBody::getRestitution() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return restitution;
    }

    void AbstractBody::setFriction(float friction) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->friction = friction;
    }

    /**
     * @return Friction (0=no friction, 1=total friction)
     */
    float AbstractBody::getFriction() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return friction;
    }

    void AbstractBody::setRollingFriction(float rollingFriction) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->rollingFriction = rollingFriction;
    }

    /**
     * @return Rolling friction (0=no friction, 1=total friction)
     */
    float AbstractBody::getRollingFriction() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return rollingFriction;
    }

    void AbstractBody::setCcdMotionThreshold(float ccdMotionThreshold) {
        std::lock_guard<std::mutex> lock(bodyMutex);

        this->ccdMotionThreshold = ccdMotionThreshold;
    }

    /**
     * @return Threshold for continuous collision detection in distance unit (process continuous collision detection if the motion in one
     * step is more then threshold). A default value is determinate automatically for each body thanks to properties 'collisionShape.ccdMotionThresholdFactor'.
     */
    float AbstractBody::getCcdMotionThreshold() const {
        std::lock_guard<std::mutex> lock(bodyMutex);

        return ccdMotionThreshold;
    }

    PairContainer* AbstractBody::getPairContainer() const {
        return nullptr;
    }

    void AbstractBody::disableAllBodies(bool value) {
        bDisableAllBodies = value;
    }

    void AbstractBody::setIsStatic(bool bIsStatic) {
        this->bIsStatic.store(bIsStatic, std::memory_order_relaxed);
        setIsActive(false);
    }

    /**
     * @return True when body is static (cannot be affected by physics world)
     */
    bool AbstractBody::isStatic() const {
        return bDisableAllBodies || bIsStatic.load(std::memory_order_relaxed);
    }

    void AbstractBody::setIsActive(bool bIsActive) {
        assert(!(bIsActive && bIsStatic)); //an active body cannot be static

        this->bIsActive = bIsActive;
    }

    /**
     * @return True when body is active (body has velocity and/or one of body in same island is active)
     */
    bool AbstractBody::isActive() const {
        return !bDisableAllBodies && bIsActive.load(std::memory_order_relaxed);
    }

    uint_fast32_t AbstractBody::getObjectId() const {
        return objectId;
    }

}
