#include <algorithm>
#include <stdexcept>

#include <scene/renderer3d/model/displayer/ModelSetDisplayer.h>
#include <graphic/render/shader/builder/ShaderBuilder.h>

namespace urchin {

    ModelSetDisplayer::ModelSetDisplayer(DisplayMode displayMode) :
            isInitialized(false),
            displayMode(displayMode),
            customModelShaderVariable(nullptr),
            depthTestEnabled(true),
            depthWriteEnabled(true),
            renderTarget(nullptr) {

    }

    ModelSetDisplayer::~ModelSetDisplayer() {
        modelsDisplayer.clear();
    }

    void ModelSetDisplayer::initialize(RenderTarget& renderTarget) {
        if (isInitialized) {
            throw std::runtime_error("Model displayer is already initialized.");
        }
        this->renderTarget = &renderTarget;

        if (displayMode == DEFAULT_MODE) {
            if (fragmentShaderName.empty()) { //use default fragment shader
                fragmentShaderName = "model.frag.spv";
            }
            modelShader = ShaderBuilder::createShader("model.vert.spv", geometryShaderName, fragmentShaderName, std::move(shaderConstants));
        } else if (displayMode == DEPTH_ONLY_MODE) {
            if (fragmentShaderName.empty()) { //use default fragment shader
                fragmentShaderName = "modelDepthOnly.frag.spv";
            }
            modelShader = ShaderBuilder::createShader("modelDepthOnly.vert.spv", geometryShaderName, fragmentShaderName, std::move(shaderConstants));
        } else if (displayMode == DIFFUSE_MODE) {
            if (fragmentShaderName.empty()) { //use default fragment shader
                fragmentShaderName = "modelDiffuse.frag.spv";
            }
            modelShader = ShaderBuilder::createShader("modelDiffuse.vert.spv", geometryShaderName, fragmentShaderName, std::move(shaderConstants));
        } else {
            throw std::invalid_argument("Unknown display mode: " + std::to_string(displayMode));
        }

        isInitialized = true;
    }

    void ModelSetDisplayer::onCameraProjectionUpdate(const Camera& camera) {
        this->projectionMatrix = camera.getProjectionMatrix();

        for (auto& modelDisplayer : modelsDisplayer) {
            modelDisplayer.second->onCameraProjectionUpdate(camera);
        }
    }

    void ModelSetDisplayer::setCustomShader(const std::string& geometryShaderName, const std::string& fragmentShaderName, std::unique_ptr<ShaderConstants> shaderConstants) {
        if (isInitialized) {
            throw std::runtime_error("Impossible to set custom shader once the model displayer initialized.");
        }

        this->geometryShaderName = geometryShaderName;
        this->fragmentShaderName = fragmentShaderName;
        this->shaderConstants = std::move(shaderConstants);
    }

    void ModelSetDisplayer::setCustomModelShaderVariable(std::unique_ptr<CustomModelShaderVariable> customModelShaderVariable) {
        this->customModelShaderVariable = std::move(customModelShaderVariable);
        modelsDisplayer.clear();
    }

    void ModelSetDisplayer::setCustomDepthOperations(bool depthTestEnabled, bool depthWriteEnabled) {
        this->depthTestEnabled = depthTestEnabled;
        this->depthWriteEnabled = depthWriteEnabled;
        modelsDisplayer.clear();
    }

    void ModelSetDisplayer::setCustomBlendFunctions(const std::vector<BlendFunction>& customBlendFunctions) {
        this->customBlendFunctions = customBlendFunctions;
        modelsDisplayer.clear();
    }

    void ModelSetDisplayer::setModels(const std::vector<Model*>& models) {
        assert(renderTarget);
        for (auto model : models) {
            const auto& itModel = modelsDisplayer.find(model);
            if (itModel == modelsDisplayer.end()) {
                auto modelDisplayer = std::make_unique<ModelDisplayer>(model, projectionMatrix, displayMode, *renderTarget, *modelShader);
                modelDisplayer->setCustomModelShaderVariable(customModelShaderVariable.get());
                modelDisplayer->setCustomDepthOperations(depthTestEnabled, depthWriteEnabled);
                modelDisplayer->setCustomBlendFunctions(customBlendFunctions);
                modelDisplayer->initialize();
                modelsDisplayer.emplace(std::make_pair(model, std::move(modelDisplayer)));
            }
        }

        this->models = models;
    }

    void ModelSetDisplayer::removeModel(Model* model) {
        modelsDisplayer.erase(model);
    }

    void ModelSetDisplayer::prepareRendering(const Matrix4<float>& viewMatrix) {
        ScopeProfiler sp(Profiler::graphic(), "modelPreRender");

        if (!isInitialized) {
            throw std::runtime_error("Model displayer must be initialized before call display");
        } else if (!renderTarget) {
            throw std::runtime_error("Render target must be specified before call display");
        }

        for (auto model : models) {
            modelsDisplayer.at(model)->prepareRendering(viewMatrix);
        }
    }

    void ModelSetDisplayer::drawBBox(const Matrix4<float>& projectionMatrix, const Matrix4<float>& viewMatrix) const {
        for (const auto& model : models) {
            modelsDisplayer.at(model)->drawBBox(projectionMatrix, viewMatrix);
        }
    }

    void ModelSetDisplayer::drawBaseBones(const Matrix4<float>& projectionMatrix, const Matrix4<float>& viewMatrix, const std::string& meshFilename) const {
        for (const auto& model : models) {
            if (model->getConstMeshes() && model->getConstMeshes()->getMeshFilename() == meshFilename) {
                modelsDisplayer.at(model)->drawBaseBones(projectionMatrix, viewMatrix);
            }
        }
    }

}
