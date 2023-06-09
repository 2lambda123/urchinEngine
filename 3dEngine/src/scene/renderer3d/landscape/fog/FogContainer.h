#pragma once

#include <stack>
#include <memory>

#include <graphics/api/GraphicsApi.h>
#include <graphics/render/GenericRendererBuilder.h>
#include <scene/renderer3d/landscape/fog/Fog.h>

namespace urchin {

    class FogContainer {
        public:
            FogContainer();

            void pushFog(const Fog*);
            void popFog();
            const Fog* getActiveFog() const;

            void setupLightingRenderer(const std::shared_ptr<GenericRendererBuilder>&) const;

            void loadFog(GenericRenderer&, std::size_t);

        private:
            std::stack<const Fog*> fogs;
            struct {
                alignas(4) bool hasFog;
                alignas(4) float density;
                alignas(4) float gradient;
                alignas(4) float maxHeight;
                alignas(16) Vector3<float> color;
            } fogData;
    };

}
