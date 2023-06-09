#pragma once

#include <UrchinCommon.h>

namespace urchin {

    /**
    * Shape used to delimit the sound
    */
    class SoundShape {
        public:
            enum ShapeType {
                SPHERE_SHAPE,
                BOX_SHAPE
            };

            explicit SoundShape(float);
            virtual ~SoundShape() = default;

            float getMargin() const;

            virtual ShapeType getShapeType() const = 0;

            virtual const Point3<float>& getCenterPosition() const = 0;
            virtual void updateCenterPosition(const Point3<float>&) = 0;

            virtual bool pointInsidePlayShape(const Point3<float>&) const = 0;
            virtual bool pointInsideStopShape(const Point3<float>&) const = 0;

            virtual std::unique_ptr<SoundShape> clone() const = 0;

        private:
            const float margin;
    };

}
