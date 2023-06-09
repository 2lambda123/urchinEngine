#pragma once

#include <model/ObjectEntity.h>

namespace urchin {

    class DefaultBodyShapeGenerator {
        public:
            explicit DefaultBodyShapeGenerator(const ObjectEntity&);

            std::unique_ptr<const CollisionShape3D> generate(CollisionShape3D::ShapeType) const;

        private:
            std::unique_ptr<ConvexHullShape3D<float>> buildConvexHullShape(const Model*) const;

            const ObjectEntity& objectEntity;
    };

}