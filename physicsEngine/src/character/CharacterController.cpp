#include <vector>
#include <limits>
#include <cmath>

#include <character/CharacterController.h>
#include <shape/CollisionCapsuleShape.h>
#include <collision/ManifoldContactPoint.h>
#include <PhysicsWorld.h>

namespace urchin {

    //static
    const float CharacterController::MAX_TIME_IN_AIR_CONSIDERED_AS_ON_GROUND = 0.2f;
    const float CharacterController::MIN_WALK_SPEED_PERCENTAGE = 0.75f;
    const float CharacterController::MAX_WALK_SPEED_PERCENTAGE = 1.25f;
    const std::array<float, 4> CharacterController::RECOVER_FACTOR = {0.4f, 0.7f, 0.9f, 1.0f};

    CharacterController::CharacterController(std::shared_ptr<PhysicsCharacter> physicsCharacter, CharacterControllerConfig config, PhysicsWorld* physicsWorld) :
            maxDepthToRecover(ConfigService::instance()->getFloatValue("character.maxDepthToRecover")),
            minUpdateFrequency(ConfigService::instance()->getFloatValue("character.minUpdateFrequency")),
            physicsCharacter(std::move(physicsCharacter)),
            config(config),
            physicsWorld(physicsWorld),
            ghostBody(nullptr),
            verticalSpeed(0.0f),
            makeJump(false),
            initialOrientation(this->physicsCharacter->getTransform().getOrientation()),
            numberOfHit(0),
            isOnGround(false),
            hitRoof(false),
            timeInTheAir(0.0f),
            jumping(false),
            slopeInPercentage(0.0f) {
        if (!physicsWorld) {
            throw std::runtime_error("Physics world cannot be null for character controller.");
        }

        //TODO review
        auto ghostBodyCapsule = std::dynamic_pointer_cast<const CollisionCapsuleShape>(this->physicsCharacter->getShape()); //TODO check if EPA algo is executed when character move
        if (ghostBodyCapsule) {
            float radius = ghostBodyCapsule->getRadius();
            float height = ghostBodyCapsule->getCylinderHeight() + (2.0f * radius);

            float updatedRadius = std::max(radius, config.getMaxHorizontalSpeed() / minUpdateFrequency);
            float updatedHeight = std::max(height, (config.getMaxVerticalSpeed() / minUpdateFrequency) * 2.0f);
            float updatedCylinderHeight = std::max(0.01f, updatedHeight - (2.0f * updatedRadius));
            auto resizedCharacterShape = std::make_shared<const CollisionCapsuleShape>(updatedRadius, updatedCylinderHeight, ghostBodyCapsule->getCapsuleOrientation());

            ghostBody = new GhostBody(this->physicsCharacter->getName(), this->physicsCharacter->getTransform(), this->physicsCharacter->getShape());
            ccdGhostBody = new GhostBody(this->physicsCharacter->getName() + "_ccd", this->physicsCharacter->getTransform(), resizedCharacterShape);
        } else {
            throw std::runtime_error("Unimplemented shape type for character controller: " + std::to_string(this->physicsCharacter->getShape()->getShapeType()));
        }

        ghostBody->setIsActive(true); //always active for get better reactivity
        ccdGhostBody->setIsActive(true); //TODO always active for get better reactivity
        physicsWorld->getCollisionWorld()->getBroadPhaseManager()->addBodyAsync(ghostBody);
        physicsWorld->getCollisionWorld()->getBroadPhaseManager()->addBodyAsync(ccdGhostBody);
    }

    CharacterController::~CharacterController() {
        physicsWorld->getCollisionWorld()->getBroadPhaseManager()->removeBodyAsync(ccdGhostBody);
        physicsWorld->getCollisionWorld()->getBroadPhaseManager()->removeBodyAsync(ghostBody);
    }

    void CharacterController::setMomentum(const Vector3<float>& momentum) {
        this->velocity = (momentum / physicsCharacter->getMass());
    }

    void CharacterController::jump() {
        makeJump = true;
    }

    /**
     * @param dt Delta of time between two simulation steps
     */
    void CharacterController::update(float dt) {
        ScopeProfiler sp(Profiler::physics(), "charactCtrlExec");

        if (!ghostBody->isStatic()) {
            //setup values
            setup(dt);

            //recover from penetration
            recoverFromPenetration(dt);

            //compute values
            slopeInPercentage = 0.0f;
            if (isOnGround) {
                verticalSpeed = 0.0f;
                slopeInPercentage = computeSlope();
            }
            if (hitRoof) {
                verticalSpeed = 0.0f;
            }

            //set new transform on character
            physicsCharacter->updateTransform(ghostBody->getTransform());
        }
    }

    void CharacterController::setup(float dt) {
        //save values
        previousBodyTransform = ghostBody->getTransform();

        //apply user move
        Point3<float> targetPosition = previousBodyTransform.getPosition();
        //TODO clamp velocity based on maximum horizontal speed
        if (isOnGround) {
            float slopeSpeedDecrease = 1.0f - (slopeInPercentage / config.getMaxSlopeInPercentage());
            slopeSpeedDecrease = MathFunction::clamp(slopeSpeedDecrease, MIN_WALK_SPEED_PERCENTAGE, MAX_WALK_SPEED_PERCENTAGE);
            targetPosition = targetPosition.translate(velocity * dt * slopeSpeedDecrease);
        } else if (timeInTheAir < config.getTimeKeepMoveInAir()) {
            float momentumSpeedDecrease = 1.0f - (timeInTheAir / config.getTimeKeepMoveInAir());
            Vector3<float> walkDirectionInAir = velocity * (1.0f - config.getPercentageControlInAir()) + velocity * config.getPercentageControlInAir();
            targetPosition = targetPosition.translate(walkDirectionInAir * dt * momentumSpeedDecrease);
        }

        //jump
        if(makeJump) {
            makeJump = false;
            bool closeToTheGround = timeInTheAir < MAX_TIME_IN_AIR_CONSIDERED_AS_ON_GROUND;
            if (closeToTheGround && !jumping) {
                verticalSpeed += config.getJumpSpeed();
                isOnGround = false;
                jumping = true;
            }
        } else if (isOnGround && jumping) {
            jumping = false;
        }

        //compute gravity velocity
        if (!isOnGround || numberOfHit > 1) {
            verticalSpeed -= (-physicsWorld->getGravity().Y) * dt;
            if (verticalSpeed < -config.getMaxVerticalSpeed()) {
                verticalSpeed = -config.getMaxVerticalSpeed();
            }
        }
        targetPosition.Y += verticalSpeed * dt;

        //CCD
        //TODO review naming + opti
        Vector3<float> moveVector = previousBodyTransform.getPosition().vector(targetPosition);
        if (moveVector.length() > ghostBody->getCcdMotionThreshold()) {
            manifoldResults.clear();
            physicsWorld->getCollisionWorld()->getNarrowPhaseManager()->processGhostBody(ccdGhostBody, manifoldResults);
            for (const auto& manifoldResult : manifoldResults) {
                for (unsigned int i = 0; i < manifoldResult.getNumContactPoints(); ++i) {
                    const ManifoldContactPoint& manifoldContactPoint = manifoldResult.getManifoldContactPoint(i);

                    Vector3<float> contactNormal = manifoldContactPoint.getNormalFromObject2();
                    Point3<float> contactPoint = manifoldContactPoint.getPointOnObject1();
                    if(manifoldResult.getBody1() == ccdGhostBody) {
                        contactPoint = manifoldContactPoint.getPointOnObject2();
                        contactNormal = -manifoldContactPoint.getNormalFromObject2();
                    }

                    float dirProportion = moveVector.normalize().dotProduct(contactNormal);
                    if(moveVector.length() * dirProportion > previousBodyTransform.getPosition().vector(contactPoint).length() * 0.8f) { //TODO update security factor
                        float forceAppliedOnObstacle = moveVector.dotProduct(contactNormal);
                        Vector3<float> obstacleReactionForce = (-contactNormal) * forceAppliedOnObstacle;
                        Vector3<float> newMove = moveVector + obstacleReactionForce;
                        targetPosition = previousBodyTransform.getPosition().translate(newMove);

                        static int count = 0;
                        std::cout<<"CCD ("<<count++<<"): " <<manifoldResult.getBody1()->getId()<<" vs. "<<manifoldResult.getBody2()->getId()<<std::endl;
                        //TODO: break or continue and review algo ?
                    }
                }
            }
        }

        //compute and apply orientation
        Quaternion<float> newOrientation = initialOrientation;
        if (!MathFunction::isZero(velocity.squareLength(), 0.001f)) {
            Quaternion<float> orientation = Quaternion<float>(velocity.normalize()).normalize();
            newOrientation = orientation * initialOrientation;
        }

        //apply transform on body
        ghostBody->setTransform(PhysicsTransform(targetPosition, newOrientation));
        ccdGhostBody->setTransform(PhysicsTransform(targetPosition, newOrientation));
    }

    void CharacterController::recoverFromPenetration(float dt) {
        resetSignificantContactValues();

        PhysicsTransform characterTransform = ghostBody->getTransform();
        for (unsigned int subStepIndex = 0; subStepIndex < RECOVER_FACTOR.size(); ++subStepIndex) {
            manifoldResults.clear();
            physicsWorld->getCollisionWorld()->getNarrowPhaseManager()->processGhostBody(ghostBody, manifoldResults);

            for (const auto& manifoldResult : manifoldResults) {
                float sign = manifoldResult.getBody1() == ghostBody ? -1.0f : 1.0f;
                for (unsigned int i = 0; i < manifoldResult.getNumContactPoints(); ++i) {
                    const ManifoldContactPoint& manifoldContactPoint = manifoldResult.getManifoldContactPoint(i);
                    float depth = manifoldContactPoint.getDepth();

                    if (depth < maxDepthToRecover) {
                        Vector3<float> normal =  manifoldContactPoint.getNormalFromObject2() * sign;
                        Vector3<float> moveVector = normal * depth * RECOVER_FACTOR[subStepIndex];

                        characterTransform.setPosition(characterTransform.getPosition().translate(moveVector));
                        ghostBody->setTransform(characterTransform);
                        ccdGhostBody->setTransform(characterTransform);

                        if (subStepIndex == 0) {
                            saveSignificantContactValues(normal);
                        }
                    }
                }
            }
        }

        computeSignificantContactValues(dt);
    }

    void CharacterController::resetSignificantContactValues() {
        significantContactValues.numberOfHit = 0;

        significantContactValues.maxDotProductUpNormalAxis = std::numeric_limits<float>::min();
        significantContactValues.mostUpVerticalNormal = Vector3<float>();

        significantContactValues.maxDotProductDownNormalAxis = -std::numeric_limits<float>::max();
        significantContactValues.mostDownVerticalNormal = Vector3<float>();
    }

    void CharacterController::saveSignificantContactValues(const Vector3<float>& normal) {
        significantContactValues.numberOfHit++;

        float dotProductUpNormalAxis = (-normal).dotProduct(Vector3<float>(0.0f, 1.0f, 0.0f));
        if (dotProductUpNormalAxis > significantContactValues.maxDotProductUpNormalAxis) {
            significantContactValues.maxDotProductUpNormalAxis = dotProductUpNormalAxis;
            significantContactValues.mostUpVerticalNormal = -normal;
        }

        float dotProductDownNormalAxis = (-normal).dotProduct(Vector3<float>(0.0f, -1.0f, 0.0f));
        if (dotProductDownNormalAxis > significantContactValues.maxDotProductDownNormalAxis) {
            significantContactValues.maxDotProductDownNormalAxis = dotProductDownNormalAxis;
            significantContactValues.mostDownVerticalNormal = -normal;
        }
    }

    void CharacterController::computeSignificantContactValues(float dt) {
        numberOfHit = significantContactValues.numberOfHit;
        isOnGround = numberOfHit > 0 && std::acos(significantContactValues.maxDotProductUpNormalAxis) < config.getMaxSlopeInRadian();
        hitRoof = numberOfHit > 0 && std::acos(significantContactValues.maxDotProductDownNormalAxis) < config.getMaxSlopeInRadian();
        timeInTheAir = isOnGround ? 0.0f : timeInTheAir + dt;
    }

    /**
     * Compute slope based on previous body position.
     * Slope is expressed in percentage. A positive value means that character climb.
     */
    float CharacterController::computeSlope() {
        Point3<float> bodyPosition = ghostBody->getTransform().getPosition();
        Point2<float> p1 = Point2<float>(bodyPosition.X, bodyPosition.Z);
        Point2<float> p2 = Point2<float>(previousBodyTransform.getPosition().X, previousBodyTransform.getPosition().Z);
        float run = p1.vector(p2).length();
        if (run == 0.0f) {
            return 0.0f;
        }

        float rise = bodyPosition.Y - previousBodyTransform.getPosition().Y;
        return rise / run;
    }
}
