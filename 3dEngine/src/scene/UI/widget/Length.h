#pragma once

#include <scene/UI/widget/LengthType.h>

namespace urchin {

    class Length {
        public:
            Length(float, LengthType);

            float getValue() const;
            LengthType getType() const;

        private:
            float value;
            LengthType type;
    };

}
