#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <UrchinCommon.h>

#include <api/render/shader/model/Shader.h>
#include <api/render/target/RenderTarget.h>
#include <scene/renderer3d/model/Model.h>
#include <scene/renderer3d/model/displayer/DisplayMode.h>
#include <scene/renderer3d/model/displayer/ModelInstanceDisplayer.h>
#include <scene/renderer3d/model/displayer/CustomModelShaderVariable.h>
#include <scene/renderer3d/camera/Camera.h>
#include <resources/geometry/GeometryContainer.h>

namespace urchin {

    /**
    * Allow to display models given in parameter depending of a configuration (view matrix, projection matrix, display mode...)
    */
    class ModelSetDisplayer {
        public:
            explicit ModelSetDisplayer(DisplayMode displayMode);
            ~ModelSetDisplayer();

            void initialize(RenderTarget&);
            void setupShader(const std::string&, const std::string&, std::unique_ptr<ShaderConstants>);
            void setupCustomShaderVariable(std::unique_ptr<CustomModelShaderVariable>);
            void setupDepthOperations(bool, bool);
            void setupFaceCull(bool);
            void setupBlendFunctions(const std::vector<BlendFunction>&);
            void setupMeshFilter(std::unique_ptr<MeshFilter>);

            void updateModels(const std::vector<Model*>&);
            void removeModel(Model*);
            const std::vector<Model*>& getModels() const;
            bool isDisplayerExist(const Model&) const;

            void prepareRendering(unsigned int, const Matrix4<float>&);

            void drawBBox(GeometryContainer&) const;
            void drawBaseBones(GeometryContainer&) const;

        private:
            ModelInstanceDisplayer* findModelInstanceDisplayer(const Model&) const;
            void clearDisplayers();
            void removeModelFromDisplayer(Model&, ModelInstanceDisplayer&);
            void addModelToDisplayer(Model&, ModelInstanceDisplayer&);

            bool isInitialized;

            std::string geometryShaderName, fragmentShaderName;
            std::unique_ptr<ShaderConstants> shaderConstants;

            DisplayMode displayMode;
            std::unique_ptr<Shader> modelShader;

            std::unique_ptr<CustomModelShaderVariable> customShaderVariable;
            bool depthTestEnabled, depthWriteEnabled;
            bool enableFaceCull;
            std::vector<BlendFunction> blendFunctions;
            std::unique_ptr<MeshFilter> meshFilter;

            RenderTarget* renderTarget;
            std::vector<Model*> models;
            std::unordered_set<ModelInstanceDisplayer*> activeModelDisplayers;
            std::unordered_map<Model*, std::unique_ptr<ModelInstanceDisplayer>> modelDisplayers;
            std::unordered_map<std::size_t, std::unique_ptr<ModelInstanceDisplayer>> modelInstanceDisplayers;
    };

}
