#pragma once

#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <mutex>
#include <UrchinCommon.h>

#include <body/model/AbstractBody.h>
#include <body/BodyContainer.h>
#include <collision/CollisionWorld.h>
#include <processable/Processable.h>
#include <processable/raytest/RayTestResult.h>
#include <visualizer/CollisionVisualizer.h>

namespace urchin {

    class PhysicsWorld {
        public:
            PhysicsWorld();
            ~PhysicsWorld();

            BodyContainer& getBodyContainer();
            CollisionWorld& getCollisionWorld();

            void addBody(std::shared_ptr<AbstractBody>);
            void removeBody(const AbstractBody&);

            void addProcessable(const std::shared_ptr<Processable>&);
            void removeProcessable(const Processable&);

            std::shared_ptr<const RayTestResult> rayTest(const Ray<float>&);

            void setGravity(const Vector3<float>&);
            Vector3<float> getGravity() const;

            void setUp(float);
            void pause();
            void unpause();
            bool isPaused() const;
            void interruptThread();
            void checkNoExceptionRaised() const;

            void createCollisionVisualizer();
            const CollisionVisualizer& getCollisionVisualizer() const;

        private:
            void startPhysicsUpdate();
            bool continueExecution() const;
            void processPhysicsUpdate(float);

            void setupProcessables(const std::vector<std::shared_ptr<Processable>>&, float, const Vector3<float>&) const;
            void executeProcessables(const std::vector<std::shared_ptr<Processable>>&, float, const Vector3<float>&) const;

            std::unique_ptr<std::jthread> physicsSimulationThread;
            std::atomic_bool physicsSimulationStopper;
            static std::exception_ptr physicsThreadExceptionPtr;

            mutable std::mutex mutex;
            Vector3<float> gravity;
            float timeStep;
            bool paused;

            BodyContainer bodyContainer;
            CollisionWorld collisionWorld;

            std::vector<std::shared_ptr<Processable>> processables;
            std::vector<std::unique_ptr<Processable>> oneShotProcessables;
            std::vector<std::shared_ptr<Processable>> copiedProcessables;

            std::unique_ptr<CollisionVisualizer> collisionVisualizer;
    };

}
