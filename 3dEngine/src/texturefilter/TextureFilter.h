#ifndef URCHINENGINE_TEXTUREFILTER_H
#define URCHINENGINE_TEXTUREFILTER_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "graphic/shader/model/Shader.h"
#include "graphic/shader/model/ShaderVar.h"
#include "graphic/render/GenericRenderer.h"

namespace urchin {

    template<class T> class TextureFilterBuilder;

    class TextureFilter {
        public:
            template<class BUILDER> explicit TextureFilter(const TextureFilterBuilder<BUILDER> *);
            virtual ~TextureFilter();

            void initialize();

            unsigned int getFboId() const;
            unsigned int getTextureID() const;

            void applyOn(unsigned int, int layersToUpdate = -1) const;

        protected:
            virtual std::string getShaderName() const = 0;
            virtual void initiateAdditionalShaderVariables(const std::unique_ptr<Shader> &);
            virtual void addFurtherTextures(const std::unique_ptr<GenericRenderer> &) const;
            virtual void completeShaderTokens(std::map<std::string, std::string> &) const = 0;

            unsigned int getTextureWidth() const;
            unsigned int getTextureHeight() const;

            const std::unique_ptr<Shader> &getTextureFilterShader() const;

            std::string toShaderVectorValues(std::vector<float> &) const;

        private:
            void initializeDisplay();
            void initializeTexture();

            bool isInitialized;

            unsigned int textureWidth, textureHeight;
            unsigned int textureType;
            unsigned int textureNumberLayer;
            int textureInternalFormat;
            unsigned int textureFormat;

            std::unique_ptr<GenericRenderer> textureRenderer;
            std::unique_ptr<Shader> textureFilterShader;
            ShaderVar layersToUpdateShaderVar;

            unsigned int fboID;
            unsigned int textureID;
    };

    #include "TextureFilter.inl"

}

#endif
