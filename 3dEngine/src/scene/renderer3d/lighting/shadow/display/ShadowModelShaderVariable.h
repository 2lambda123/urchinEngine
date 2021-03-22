#ifndef URCHINENGINE_SHADOWMODELUNIFORM_H
#define URCHINENGINE_SHADOWMODELUNIFORM_H

#include "UrchinCommon.h"

#include "scene/renderer3d/lighting/shadow/light/LightShadowMap.h"
#include "scene/renderer3d/model/Model.h"
#include "scene/renderer3d/model/displayer/CustomModelShaderVariable.h"
#include "graphic/render/shader/model/Shader.h"

namespace urchin {

    /**
    * Load custom model shader variables for shadow model displayer
    */
    class ShadowModelShaderVariable : public CustomModelShaderVariable {
        public:
            explicit ShadowModelShaderVariable(const LightShadowMap*);

            void setupMeshRenderer(const std::unique_ptr<GenericRendererBuilder>&) override;
            void loadCustomShaderVariables(const std::unique_ptr<GenericRenderer>&) override;

        private:
            void refreshShaderVariables();

            const LightShadowMap* lightShadowMap;

            std::vector<Matrix4<float>> projectionMatrices;
            int layerToUpdate;
    };

}

#endif
