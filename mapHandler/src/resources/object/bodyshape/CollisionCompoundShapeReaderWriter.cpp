#include <vector>

#include "CollisionCompoundShapeReaderWriter.h"
#include "CollisionShapeReaderWriterRetriever.h"
#include <resources/common/OrientationReaderWriter.h>

namespace urchin {

    CollisionShape3D* CollisionCompoundShapeReaderWriter::loadFrom(const DataChunk* mainShapeChunk, const DataParser& dataParser) const {
        auto localizedShapesListChunk = dataParser.getUniqueChunk(true, LOCALIZED_SHAPES, UdaAttribute(), mainShapeChunk);
        auto localizedShapesChunk = dataParser.getChunks(LOCALIZED_SHAPE, UdaAttribute(), localizedShapesListChunk);

        std::vector<std::shared_ptr<const LocalizedCollisionShape>> compoundShapes;
        for (std::size_t i = 0; i < localizedShapesChunk.size(); ++i) {
            std::shared_ptr<LocalizedCollisionShape> localizedShape(new LocalizedCollisionShape());

            localizedShape->position = i;

            loadTransformOn(*localizedShape, localizedShapesChunk[i], dataParser);

            auto shapeChunk = dataParser.getUniqueChunk(true, COMPOUND_SHAPE_TAG, UdaAttribute(), localizedShapesChunk[i]);
            CollisionShape3D* embeddedCollisionShape = CollisionShapeReaderWriterRetriever::retrieveShapeReaderWriter(shapeChunk)->loadFrom(shapeChunk, dataParser);
            localizedShape->shape = std::unique_ptr<CollisionShape3D>(embeddedCollisionShape);

            compoundShapes.push_back(std::move(localizedShape));
        }

        return new CollisionCompoundShape(std::move(compoundShapes));
    }

    void CollisionCompoundShapeReaderWriter::writeOn(DataChunk& mainShapeChunk, const CollisionShape3D& mainCollisionShape, UdaWriter& udaWriter) const {
        mainShapeChunk.addAttribute(UdaAttribute(TYPE_ATTR, COMPOUND_SHAPE_VALUE));

        const auto& compoundShape = dynamic_cast<const CollisionCompoundShape&>(mainCollisionShape);

        auto& localizedShapesListChunk = udaWriter.createChunk(LOCALIZED_SHAPES, UdaAttribute(), &mainShapeChunk);
        const std::vector<std::shared_ptr<const LocalizedCollisionShape>>& localizedShapes = compoundShape.getLocalizedShapes();
        for (const auto& localizedShape : localizedShapes) {
            auto& localizedShapeChunk = udaWriter.createChunk(LOCALIZED_SHAPE, UdaAttribute(), &localizedShapesListChunk);

            writeTransformOn(localizedShapeChunk, *localizedShape, udaWriter);

            auto& shapeChunk = udaWriter.createChunk(COMPOUND_SHAPE_TAG, UdaAttribute(), &localizedShapeChunk);
            CollisionShapeReaderWriterRetriever::retrieveShapeReaderWriter(*localizedShape->shape)->writeOn(shapeChunk, *localizedShape->shape, udaWriter);
        }
    }

    void CollisionCompoundShapeReaderWriter::loadTransformOn(LocalizedCollisionShape& localizedShape, const DataChunk* localizedShapeChunk, const DataParser& dataParser) {
        auto transformChunk = dataParser.getUniqueChunk(true, TRANSFORM_TAG, UdaAttribute(), localizedShapeChunk);

        auto positionChunk = dataParser.getUniqueChunk(true, POSITION_TAG, UdaAttribute(), transformChunk);
        Point3<float> position = positionChunk->getPoint3Value();

        Quaternion<float> orientation = OrientationReaderWriter::loadOrientation(transformChunk, dataParser);

        localizedShape.transform = PhysicsTransform(position, orientation);
    }

    void CollisionCompoundShapeReaderWriter::writeTransformOn(DataChunk& localizedShapeChunk, const LocalizedCollisionShape& localizedShape, UdaWriter& udaWriter) {
        auto& transformChunk = udaWriter.createChunk(TRANSFORM_TAG, UdaAttribute(), &localizedShapeChunk);

        auto& positionChunk = udaWriter.createChunk(POSITION_TAG, UdaAttribute(), &transformChunk);
        positionChunk.setPoint3Value(localizedShape.transform.getPosition());

        OrientationReaderWriter::writeOrientation(transformChunk, localizedShape.transform.getOrientation(), udaWriter);
    }

}
