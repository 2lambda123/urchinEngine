#include <stdexcept>

#include "TextureFilter.h"
#include "graphic/render/shader/builder/ShaderBuilder.h"
#include "graphic/render/GenericRendererBuilder.h"

namespace urchin {

    void TextureFilter::initialize() {
        if (isInitialized) {
            throw std::runtime_error("Texture filter is already initialized");
        }

        initializeTexture();
        initializeDisplay();

        isInitialized = true;
    }

    void TextureFilter::initializeTexture() {
        if (textureType == TextureType::DEFAULT) {
            texture = Texture::build(textureWidth, textureHeight, textureFormat, nullptr);
        } else if (textureType == TextureType::ARRAY) {
            texture = Texture::buildArray(textureWidth, textureHeight, textureNumberLayer, textureFormat, nullptr);
        } else {
            throw std::invalid_argument("Unsupported texture type for filter: " + std::to_string(textureType));
        }

        offscreenRenderTarget = std::make_unique<OffscreenRender>(RenderTarget::NO_DEPTH_ATTACHMENT);
        offscreenRenderTarget->onResize();
        offscreenRenderTarget->addTexture(texture);
    }

    void TextureFilter::initializeDisplay() {
        std::locale::global(std::locale("C")); //for float

        std::map<std::string, std::string> shaderTokens;
        if (textureFormat == TextureFormat::RG_32_FLOAT) {
            shaderTokens["OUTPUT_TYPE"] = "vec2";
            shaderTokens["SOURCE_TEX_COMPONENTS"] = "rg";
        } else if (textureFormat == TextureFormat::GRAYSCALE_16_FLOAT) {
            shaderTokens["OUTPUT_TYPE"] = "float";
            shaderTokens["SOURCE_TEX_COMPONENTS"] = "r";
        } else {
            throw std::invalid_argument("Unsupported texture format for filter: " + std::to_string(textureFormat));
        }

        this->completeShaderTokens(shaderTokens);

        if (textureType == TextureType::ARRAY) {
            shaderTokens["MAX_VERTICES"] = std::to_string(3 * textureNumberLayer);
            shaderTokens["NUMBER_LAYER"] = std::to_string(textureNumberLayer);

            textureFilterShader = ShaderBuilder::createShader("textureFilter.vert", "textureFilter.geom", getShaderName() + "Array.frag", shaderTokens);
        } else if (textureType == TextureType::DEFAULT) {
            textureFilterShader = ShaderBuilder::createShader("textureFilter.vert", "", getShaderName() + ".frag", shaderTokens);
        } else {
            throw std::invalid_argument("Unsupported texture type for filter: " + std::to_string(textureType));
        }

        int layersToUpdate = 0;

        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(-1.0f, 1.0f), Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, -1.0f),
                Point2<float>(-1.0f, 1.0f), Point2<float>(1.0f, -1.0f), Point2<float>(-1.0f, -1.0f)
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 1.0f), Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, 0.0f),
                Point2<float>(0.0f, 1.0f), Point2<float>(1.0f, 0.0f), Point2<float>(0.0f, 0.0f)
        };
        auto textureRendererBuilder = std::make_unique<GenericRendererBuilder>(offscreenRenderTarget, getTextureFilterShader(), ShapeType::TRIANGLE);
        textureRendererBuilder
                ->addData(vertexCoord)
                ->addData(textureCoord)
                ->addShaderData(sizeof(layersToUpdate), &layersToUpdate) //binding 0
                ->addTextureReader(TextureReader::build(sourceTexture, TextureParam::buildLinear()));
        initiateAdditionalDisplay(textureRendererBuilder);

        textureRenderer = textureRendererBuilder->build();
    }

    void TextureFilter::initiateAdditionalDisplay(const std::unique_ptr<GenericRendererBuilder>&) {
        //do nothing: to override
    }

    const std::shared_ptr<Texture>& TextureFilter::getTexture() const {
        return texture;
    }

    unsigned int TextureFilter::getTextureWidth() const {
        return textureWidth;
    }

    unsigned int TextureFilter::getTextureHeight() const {
        return textureHeight;
    }

    const std::shared_ptr<Shader>& TextureFilter::getTextureFilterShader() const {
        return textureFilterShader;
    }

    const std::unique_ptr<GenericRenderer>& TextureFilter::getTextureRenderer() const {
        return textureRenderer;
    }

    std::string TextureFilter::toShaderVectorValues(std::vector<float>& vector) {
        std::string vectorValuesStr;
        for (std::size_t i = 0; i < vector.size(); ++i) {
            vectorValuesStr += std::to_string(vector[i]);
            if (i != vector.size() - 1) {
                vectorValuesStr += ", ";
            }
        }

        return vectorValuesStr;
    }

    /**
     * @param layersToUpdate Specify the layers which must be affected by the filter (only for TextureFormat::ARRAY).
     * Lowest bit represent the first layer, the second lowest bit represent the second layer, etc.
     */
    void TextureFilter::applyFilter(int layersToUpdate) const {
        if (!isInitialized) {
            throw std::runtime_error("Texture filter must be initialized before apply.");
        }

        if (textureType == TextureType::ARRAY) {
            textureRenderer->updateShaderData(0, &layersToUpdate);
        }

        //TODO offscreenRenderTarget->display(textureRenderer);
    }
}
