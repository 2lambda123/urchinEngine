#include <stdexcept>

#include "BilateralBlurFilter.h"
#include "texture/filter/bilateralblur/BilateralBlurFilterBuilder.h"
#include "graphic/render/shader/builder/ShaderBuilder.h"
#include "graphic/render/GenericRendererBuilder.h"

namespace urchin {

    BilateralBlurFilter::BilateralBlurFilter(const BilateralBlurFilterBuilder* textureFilterBuilder, BlurDirection blurDirection):
            TextureFilter(textureFilterBuilder),
            depthTexture(textureFilterBuilder->getDepthTexture()),
            blurDirection(blurDirection),
            blurSize(textureFilterBuilder->getBlurSize()),
            blurSharpness(textureFilterBuilder->getBlurSharpness()),
            textureSize((BlurDirection::VERTICAL == blurDirection) ? getTextureHeight() : getTextureWidth()),
            cameraPlanes({}) {
        if (blurSize <= 1) {
            throw std::invalid_argument("Blur size must be greater than one. Value: " + std::to_string(blurSize));
        } else if (blurSize % 2 == 0) {
            throw std::invalid_argument("Blur size must be an odd number. Value: " + std::to_string(blurSize));
        }

        std::vector<float> offsets = computeOffsets();
        offsetsTab = toShaderVectorValues(offsets);
    }

    void BilateralBlurFilter::onCameraProjectionUpdate(float nearPlane, float farPlane) {
        cameraPlanes.nearPlane = nearPlane;
        cameraPlanes.farPlane = farPlane;

        getTextureRenderer()->updateShaderData(1, &cameraPlanes);
    }

    std::string BilateralBlurFilter::getShaderName() const {
        if(getTextureFormat() == TextureFormat::GRAYSCALE_16_FLOAT && getTextureType() == TextureType::DEFAULT) {
            return "texFilterBilateralBlur";
        }
        throw std::runtime_error("Unimplemented bilateral blur filter for: " + std::to_string(getTextureFormat()) + " - " + std::to_string(getTextureType()));
    }

    void BilateralBlurFilter::initiateAdditionalDisplay(const std::shared_ptr<GenericRendererBuilder>& textureRendererBuilder) {
        textureRendererBuilder
                ->addShaderData(sizeof(cameraPlanes), &cameraPlanes) //binding 1
                ->addTextureReader(TextureReader::build(depthTexture, TextureParam::buildNearest()));
    }

    std::unique_ptr<ShaderConstants> BilateralBlurFilter::buildShaderConstants() const {
        BilateralBlurShaderConst bilateralBlurData{};
        bilateralBlurData.numberLayer = getTextureLayer();
        bilateralBlurData.isVerticalBlur = blurDirection == BlurDirection::VERTICAL;
        bilateralBlurData.kernelRadius = blurSize / 2;
        bilateralBlurData.blurSharpness = blurSharpness;
        bilateralBlurData.offsets = computeOffsets();
        std::vector<std::size_t> variablesSize = {
                sizeof(BilateralBlurShaderConst::numberLayer),
                sizeof(BilateralBlurShaderConst::isVerticalBlur),
                sizeof(BilateralBlurShaderConst::kernelRadius),
                sizeof(BilateralBlurShaderConst::blurSharpness),
                sizeof(float) * bilateralBlurData.offsets.size()
        };
        return std::make_unique<ShaderConstants>(variablesSize, &bilateralBlurData);
    }

    std::vector<float> BilateralBlurFilter::computeOffsets() const {
        unsigned int numOffsets = blurSize / 2;
        std::vector<float> offsets(numOffsets, 0.0f);

        if (textureSize != 0) {
            float pixelSize = 1.0f / (float)textureSize;
            for (unsigned int i = 1; i <= numOffsets; ++i) {
                offsets[i - 1] = pixelSize * (float)i;
            }
        }

        return offsets;
    }
}
