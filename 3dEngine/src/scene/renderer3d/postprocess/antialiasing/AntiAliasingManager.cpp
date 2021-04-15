#include <map>
#include <string>

#include "scene/renderer3d/postprocess/antialiasing/AntiAliasingManager.h"
#include "graphic/render/shader/builder/ShaderBuilder.h"
#include "graphic/render/GenericRendererBuilder.h"

#define DEFAULT_AA_QUALITY AntiAliasingManager::Quality::VERY_HIGH

namespace urchin {

    AntiAliasingManager::AntiAliasingManager(std::shared_ptr<RenderTarget> renderTarget) :
            renderTarget(std::move(renderTarget)),
            quality(DEFAULT_AA_QUALITY) {
        loadFxaaShader();
    }

    void AntiAliasingManager::loadFxaaShader() {
        int qualityInt = (int)quality;
        std::vector<std::size_t> variablesSize = {sizeof(qualityInt),};
        auto shaderConstants = std::make_unique<ShaderConstants>(variablesSize, &qualityInt);

        fxaaShader = ShaderBuilder::createShader("fxaa.vert.spv", "", "fxaa.frag.spv", std::move(shaderConstants));
    }

    void AntiAliasingManager::onTextureUpdate(const std::shared_ptr<Texture>& texture) {
        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(-1.0f, -1.0f), Point2<float>(1.0f, -1.0f), Point2<float>(1.0f, 1.0f),
                Point2<float>(-1.0f, -1.0f), Point2<float>(1.0f, 1.0f), Point2<float>(-1.0f, 1.0f)
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 0.0f), Point2<float>(1.0f, 1.0f),
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 1.0f), Point2<float>(0.0f, 1.0f)
        };
        renderer = GenericRendererBuilder::create("anti aliasing", renderTarget, fxaaShader, ShapeType::TRIANGLE)
                ->addData(vertexCoord)
                ->addData(textureCoord)
                ->addUniformData(sizeof(invSceneSize), &invSceneSize) //binding 0
                ->addUniformTextureReader(TextureReader::build(texture, TextureParam::buildLinear())) //binding 1
                ->build();
    }

    void AntiAliasingManager::onResize(unsigned int sceneWidth, unsigned int sceneHeight) {
        invSceneSize = Point2<float>(1.0f / (float)sceneWidth, 1.0f / (float)sceneHeight);
        if (renderer) {
            renderer->updateUniformData(0, &invSceneSize);
        }
    }

    void AntiAliasingManager::setQuality(Quality quality) {
        this->quality = quality;

        loadFxaaShader();
    }

    void AntiAliasingManager::applyAntiAliasing() {
        if (renderer) {
            renderer->addOnRenderTarget();
        }
    }

}
