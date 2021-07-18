#pragma once

#include <UrchinCommon.h>
#include <vector>

#include <body/BodyContainer.h>
#include <collision/OverlappingPair.h>

namespace urchin {

    /**
    * Manager allowing to perform integration on bodies velocity
    */
    class IntegrateVelocity {
        public:
            explicit IntegrateVelocity(const BodyContainer&);

            void process(float, const std::vector<std::unique_ptr<OverlappingPair>>&, const Vector3<float>&);

        private:
            void applyGravityForce(const Vector3<float>&, float);
            void applyRollingFrictionResistanceForce(float , const std::vector<std::unique_ptr<OverlappingPair>>&);

            const BodyContainer& bodyContainer;
    };

}
