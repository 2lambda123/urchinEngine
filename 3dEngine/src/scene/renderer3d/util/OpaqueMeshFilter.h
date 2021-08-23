#pragma once

#include <scene/renderer3d/model/displayer/MeshFilter.h>

namespace urchin {

    class OpaqueMeshFilter : public MeshFilter {
        public:
            bool isAccepted(const ConstMesh&) const override;
    };

}