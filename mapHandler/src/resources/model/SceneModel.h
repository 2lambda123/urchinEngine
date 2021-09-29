#pragma once

#include <string>
#include <Urchin3dEngine.h>
#include <UrchinPhysicsEngine.h>
#include <UrchinAIEngine.h>

#include <resources/common/SceneEntity.h>

namespace urchin {

    /**
    * Represent an models on the scene (3d model and physics)
    */
    class SceneModel : public SceneEntity {
        public:
            friend class Map;

            SceneModel();
            ~SceneModel() override;

            void setup(Renderer3d*, PhysicsWorld*, AIEnvironment*);

            const std::string& getName() const;
            void setName(const std::string&);

            Model* getModel() const;
            void setModel(const std::shared_ptr<Model>&);

            void setupInteractiveBody(const std::shared_ptr<RigidBody>&);

            RigidBody* getRigidBody() const override;

        protected:
            void moveTo(const Point3<float>&, const Quaternion<float>&) override;

        private:
            void loadFrom(const UdaChunk*, const UdaParser&);
            void writeOn(UdaChunk&, UdaWriter&) const;

            void setupRigidBody(const std::shared_ptr<RigidBody>&);
            void setupAIObject();

            void deleteRigidBody();
            void deleteAIObjects();

            static constexpr char MODEL_TAG[] = "model";
            static constexpr char PHYSICS_TAG[] = "physics";
            static constexpr char NAME_ATTR[] = "name";

            Renderer3d* renderer3d;
            PhysicsWorld* physicsWorld;
            AIEnvironment* aiEnvironment;

            std::string name;
            std::shared_ptr<Model> model;
            std::shared_ptr<RigidBody> rigidBody;
            std::shared_ptr<AIObject> aiObject;
    };

}