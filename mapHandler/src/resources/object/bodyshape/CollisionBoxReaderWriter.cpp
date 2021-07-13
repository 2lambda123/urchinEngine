#include "CollisionBoxReaderWriter.h"

namespace urchin {

    CollisionShape3D* CollisionBoxReaderWriter::loadFrom(const DataChunk* shapeChunk, const DataParser& dataParser) const {
        auto halfSizeChunk = dataParser.getUniqueChunk(true, HALF_SIZE_TAG, UdaAttribute(), shapeChunk);
        Vector3<float> halfSize = halfSizeChunk->getVector3Value();

        return new CollisionBoxShape(halfSize);
    }

    void CollisionBoxReaderWriter::writeOn(DataChunk& shapeChunk, const CollisionShape3D& collisionShape, UdaWriter& udaWriter) const {
        shapeChunk.addAttribute(UdaAttribute(TYPE_ATTR, BOX_VALUE));

        const auto& boxShape = dynamic_cast<const CollisionBoxShape&>(collisionShape);

        auto& halfSizeChunk = udaWriter.createChunk(HALF_SIZE_TAG, UdaAttribute(), &shapeChunk);
        halfSizeChunk.setVector3Value(boxShape.getHalfSizes());
    }

}
