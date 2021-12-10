#include <stdexcept>
#include <AIEnvironment.h>

#include <resources/model/ObjectEntity.h>
#include <resources/model/ModelReaderWriter.h>
#include <resources/model/RigidBodyReaderWriter.h>
#include <resources/common/AIEntityBuilder.h>
#include <resources/common/TagsReaderWriter.h>

namespace urchin {

    ObjectEntity::ObjectEntity() :
            renderer3d(nullptr),
            physicsWorld(nullptr),
            aiEnvironment(nullptr),
            rigidBody(nullptr),
            aiObject(nullptr) {

    }

    ObjectEntity::~ObjectEntity() {
        renderer3d->removeModel(model.get());
        deleteRigidBody();
        deleteAIObjects();
    }

    void ObjectEntity::setup(Renderer3d* renderer3d, PhysicsWorld* physicsWorld, AIEnvironment* aiEnvironment) {
        if (this->renderer3d) {
            throw std::invalid_argument("Cannot add the scene object on two different object managers.");
        }
        if (!renderer3d) {
            throw std::invalid_argument("Cannot specify a null renderer 3d for a scene object.");
        }

        this->renderer3d = renderer3d;
        this->physicsWorld = physicsWorld;
        this->aiEnvironment = aiEnvironment;

        renderer3d->addModel(model);

        if (physicsWorld && rigidBody) {
            physicsWorld->addBody(rigidBody);
        }

        if (aiEnvironment && aiObject) {
            aiEnvironment->addEntity(aiObject);
        }
    }

    void ObjectEntity::loadFrom(const UdaChunk* chunk, const UdaParser& udaParser) {
        this->name = chunk->getAttributeValue(NAME_ATTR);

        auto modelChunk = udaParser.getUniqueChunk(true, MODEL_TAG, UdaAttribute(), chunk);
        setModel(ModelReaderWriter::loadFrom(modelChunk, udaParser));

        auto physicsChunk = udaParser.getUniqueChunk(false, PHYSICS_TAG, UdaAttribute(), chunk);
        if (physicsChunk != nullptr) {
            std::string rigidBodyId = this->name;
            const Transform<float>& modelTransform = this->model->getTransform();
            setupInteractiveBody(RigidBodyReaderWriter::loadFrom(physicsChunk, rigidBodyId, modelTransform, udaParser));
        }

        auto tagsChunk = udaParser.getUniqueChunk(false, TAGS_TAG, UdaAttribute(), chunk);
        if (tagsChunk != nullptr) {
            addTags(TagsReaderWriter::loadTags(tagsChunk, udaParser));
        }
    }

    void ObjectEntity::writeOn(UdaChunk& chunk, UdaWriter& udaWriter) const {
        chunk.addAttribute(UdaAttribute(NAME_ATTR, this->name));

        auto& modelChunk = udaWriter.createChunk(MODEL_TAG, UdaAttribute(), &chunk);
        ModelReaderWriter::writeOn(modelChunk, *model, udaWriter);

        if (rigidBody) {
            auto& physicsChunk = udaWriter.createChunk(PHYSICS_TAG, UdaAttribute(), &chunk);
            RigidBodyReaderWriter::writeOn(physicsChunk, *rigidBody, udaWriter);
        }

        if (!getTags().empty()) {
            auto& tagsChunk = udaWriter.createChunk(TAGS_TAG, UdaAttribute(), &chunk);
            TagsReaderWriter::writeTags(tagsChunk, getTags(), udaWriter);
        }
    }

    const std::string& ObjectEntity::getName() const {
        return name;
    }

    void ObjectEntity::setName(const std::string& name) {
        this->name = name;
    }

    Model* ObjectEntity::getModel() const {
        return model.get();
    }

    void ObjectEntity::setModel(const std::shared_ptr<Model>& model) {
        if (!model) {
            throw std::invalid_argument("Cannot set a null model on scene object.");
        }

        if (renderer3d) {
            renderer3d->removeModel(this->model.get());
            renderer3d->addModel(model);
        }

        this->model = model;
    }

    void ObjectEntity::setupInteractiveBody(const std::shared_ptr<RigidBody>& rigidBody) {
        setupRigidBody(rigidBody);
        setupAIObject();
    }

    RigidBody* ObjectEntity::getRigidBody() const {
        return rigidBody.get();
    }

    void ObjectEntity::setupRigidBody(const std::shared_ptr<RigidBody>& rigidBody) {
        deleteRigidBody();

        this->rigidBody = rigidBody;
        if (physicsWorld && rigidBody) {
            physicsWorld->addBody(rigidBody);
        }
    }

    void ObjectEntity::setupAIObject() {
        deleteAIObjects();

        if (!rigidBody || !model->getConstMeshes()) {
            this->aiObject = nullptr;
        } else {
            std::string aiObjectName = "#" + rigidBody->getId(); //prefix to avoid collision name with terrains
            this->aiObject = AIEntityBuilder::buildAIObject(aiObjectName, rigidBody->getShape(), rigidBody->getTransform().toTransform());
            if (aiEnvironment) {
                aiEnvironment->addEntity(aiObject);
            }
        }
    }

    void ObjectEntity::deleteRigidBody() {
        if (physicsWorld && rigidBody) {
            physicsWorld->removeBody(*rigidBody);
        }
        rigidBody = nullptr;
    }

    void ObjectEntity::deleteAIObjects() {
        if (aiEnvironment && aiObject) {
            aiEnvironment->removeEntity(aiObject);
        }
    }

    void ObjectEntity::refresh() {
        if (rigidBody && rigidBody->isActive()) {
            PhysicsTransform physicsTransform = rigidBody->getTransform();
            model->setTransform(Transform(physicsTransform.getPosition(), physicsTransform.getOrientation(), model->getTransform().getScale()));
            if (aiObject) {
                aiObject->updateTransform(physicsTransform.getPosition(), physicsTransform.getOrientation());
            }
        }
    }

}