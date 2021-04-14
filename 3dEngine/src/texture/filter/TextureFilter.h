#ifndef URCHINENGINE_TEXTUREFILTER_H
#define URCHINENGINE_TEXTUREFILTER_H

#include <string>
#include <memory>
#include <map>
#include <vector>

#include "graphic/render/shader/model/Shader.h"
#include "graphic/render/GenericRenderer.h"
#include "graphic/render/target/OffscreenRender.h"

namespace urchin {

    template<class T> class TextureFilterBuilder;

    class TextureFilter {
        public:
            template<class BUILDER> explicit TextureFilter(const TextureFilterBuilder<BUILDER>*);
            virtual ~TextureFilter();

            void initialize(const std::string&);

            const std::shared_ptr<Texture>& getTexture() const;

            void applyFilter(int layersToUpdate = -1) const;

        protected:
            virtual std::string getShaderName() const = 0;
            virtual void initiateAdditionalDisplay(const std::shared_ptr<GenericRendererBuilder>&);
            virtual std::unique_ptr<ShaderConstants> buildShaderConstants() const = 0;

            TextureFormat getTextureFormat() const;
            TextureType getTextureType() const;
            unsigned int getTextureWidth() const;
            unsigned int getTextureHeight() const;
            unsigned int getTextureLayer() const;

            const std::shared_ptr<Shader>& getTextureFilterShader() const;
            const std::shared_ptr<GenericRenderer>& getTextureRenderer() const;

            static std::string toShaderVectorValues(std::vector<float>&); //TODO remove..

        private:
            void initializeTexture(const std::string&);
            void initializeDisplay(const std::string&);

            static constexpr unsigned int TEXTURE_LAYER_SHADER_LIMIT = 10; //must be equals to 'TEXTURE_LAYER_SHADER_LIMIT' in texFilter shaders
            bool isInitialized;

            //source texture
            std::shared_ptr<Texture> sourceTexture;

            //target texture
            unsigned int textureWidth, textureHeight;
            TextureType textureType;
            unsigned int textureNumberLayer;
            TextureFormat textureFormat;

            std::shared_ptr<OffscreenRender> offscreenRenderTarget;
            std::shared_ptr<Texture> texture;

            std::shared_ptr<GenericRenderer> textureRenderer;
            std::shared_ptr<Shader> textureFilterShader;
    };

    #include "TextureFilter.inl"

}

#endif
