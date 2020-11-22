#include <GL/glew.h>
#include <stdexcept>

#include "TextureFilter.h"
#include "graphic/shader/builder/ShaderBuilder.h"
#include "graphic/shader/data/ShaderDataSender.h"
#include "graphic/render/GenericRendererBuilder.h"

namespace urchin {

    TextureFilter::~TextureFilter() {
        glDeleteFramebuffers(1, &fboID);
        glDeleteTextures(1, &textureID);
    }

    void TextureFilter::initialize() {
        if (isInitialized) {
            throw std::runtime_error("Texture filter is already initialized");
        }

        initializeDisplay();
        initializeTexture();

        isInitialized = true;
    }

    void TextureFilter::initializeDisplay() {
        std::locale::global(std::locale("C")); //for float

        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(-1.0f, 1.0f), Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, -1.0f),
                Point2<float>(-1.0f, 1.0f), Point2<float>(1.0f, -1.0f), Point2<float>(-1.0f, -1.0f)
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 1.0f), Point2<float>(1.0f, 1.0f), Point2<float>(1.0f, 0.0f),
                Point2<float>(0.0f, 1.0f), Point2<float>(1.0f, 0.0f), Point2<float>(0.0f, 0.0f)
        };
        textureRenderer = std::make_unique<GenericRendererBuilder>(ShapeType::TRIANGLE)
                ->addData(&vertexCoord)
                ->addData(&textureCoord)
                ->build();

        std::map<std::string, std::string> shaderTokens;
        if (textureFormat == GL_RGB) {
            shaderTokens["OUTPUT_TYPE"] = "vec3";
            shaderTokens["SOURCE_TEX_COMPONENTS"] = "rgb";
        } else if (textureFormat == GL_RG) {
            shaderTokens["OUTPUT_TYPE"] = "vec2";
            shaderTokens["SOURCE_TEX_COMPONENTS"] = "rg";
        } else if (textureFormat == GL_RED) {
            shaderTokens["OUTPUT_TYPE"] = "float";
            shaderTokens["SOURCE_TEX_COMPONENTS"] = "r";
        } else {
            throw std::invalid_argument("Unsupported texture format for filter: " + std::to_string(textureFormat));
        }

        this->completeShaderTokens(shaderTokens);

        if (textureType == TextureType::ARRAY) {
            shaderTokens["MAX_VERTICES"] = std::to_string(3*textureNumberLayer);
            shaderTokens["NUMBER_LAYER"] = std::to_string(textureNumberLayer);

            textureFilterShader = ShaderBuilder().createShader("textureFilter.vert", "textureFilter.geom", getShaderName() + "Array.frag", shaderTokens);
        } else if (textureType == TextureType::DEFAULT) {
            textureFilterShader = ShaderBuilder().createShader("textureFilter.vert", "", getShaderName() + ".frag", shaderTokens);
        } else {
            throw std::invalid_argument("Unsupported texture type for filter: " + std::to_string(textureType));
        }

        int texUnit = 0;
        ShaderDataSender().sendData(ShaderVar(textureFilterShader, "tex"), texUnit);
        layersToUpdateShaderVar = ShaderVar(textureFilterShader, "layersToUpdate");
        initiateAdditionalShaderVariables(textureFilterShader);
    }

    void TextureFilter::initializeTexture() {
        glGenFramebuffers(1, &fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);

        GLenum fboAttachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, fboAttachments);
        glReadBuffer(GL_NONE);

        glGenTextures(1, &textureID);
        glBindTexture(textureType==TextureType::DEFAULT ? GL_TEXTURE_2D : GL_TEXTURE_2D_ARRAY, textureID);
        if (textureType == TextureType::ARRAY) {
            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, textureInternalFormat, textureWidth, textureHeight, textureNumberLayer, 0, textureFormat, GL_FLOAT, nullptr);
        } else if (textureType == TextureType::DEFAULT) {
            glTexImage2D(GL_TEXTURE_2D, 0, textureInternalFormat, textureWidth, textureHeight, 0, textureFormat, GL_FLOAT, nullptr);
        } else {
            throw std::invalid_argument("Unsupported texture type for filter: " + std::to_string(textureType));
        }
        glFramebufferTexture(GL_FRAMEBUFFER, fboAttachments[0], textureID, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void TextureFilter::initiateAdditionalShaderVariables(const std::unique_ptr<Shader> &) {
        //do nothing: to override
    }

    void TextureFilter::addFurtherTextures(const std::unique_ptr<GenericRenderer> &) const {
        //do nothing: to override
    }

    unsigned int TextureFilter::getFboId() const {
        return fboID;
    }

    unsigned int TextureFilter::getTextureID() const {
        return textureID;
    }

    unsigned int TextureFilter::getTextureWidth() const {
        return textureWidth;
    }

    unsigned int TextureFilter::getTextureHeight() const {
        return textureHeight;
    }

    const std::unique_ptr<Shader> &TextureFilter::getTextureFilterShader() const {
        return textureFilterShader;
    }

    std::string TextureFilter::toShaderVectorValues(std::vector<float> &vector) const {
        std::string vectorValuesStr;
        for (std::size_t i=0;i<vector.size(); ++i) {
            vectorValuesStr += std::to_string(vector[i]);
            if (i!=vector.size()-1) {
                vectorValuesStr += ", ";
            }
        }

        return vectorValuesStr;
    }

    /**
     * @param layersToUpdate Specify the layers which must be affected by the filter (only for GL_TEXTURE_2D_ARRAY).
     * Lowest bit represent the first layer, the second lowest bit represent the second layer, etc.
     */
    void TextureFilter::applyOn(unsigned int sourceTextureId, int layersToUpdate) const {
        if (!isInitialized) {
            throw std::runtime_error("Texture filter must be initialized before apply.");
        }

        textureFilterShader->bind();

        TextureType sourceTextureType = layersToUpdate == -1 ? TextureType::DEFAULT : TextureType::ARRAY;
        textureRenderer->clearAdditionalTextures();
        textureRenderer->addAdditionalTexture(TextureReader::build(sourceTextureId, sourceTextureType, TextureParam::buildLinear()));
        addFurtherTextures(textureRenderer);

        if (textureType == TextureType::ARRAY) {
            assert(layersToUpdate != -1);
            ShaderDataSender().sendData(layersToUpdateShaderVar, static_cast<unsigned int>(layersToUpdate));
        }

        glViewport(0, 0, textureWidth, textureHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, fboID);

        textureRenderer->draw();
    }
}
