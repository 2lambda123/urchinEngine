#ifndef URCHINENGINE_GENERICDISPLAYERBUILDER_H
#define URCHINENGINE_GENERICDISPLAYERBUILDER_H

#include <memory>
#include <vector>

#include "graphic/displayer/generic/GenericDisplayer.h"
#include "graphic/texture/Texture.h"

namespace urchin {

    class GenericDisplayerBuilder {
        public:
            GenericDisplayerBuilder();

            GenericDisplayerBuilder *numberOfQuad(unsigned int);
            unsigned int getNumberOfQuad() const;

            GenericDisplayerBuilder *dimension(unsigned int);
            unsigned int getDimension() const;

            GenericDisplayerBuilder *vertexData(unsigned int, void *, bool);
            unsigned int getVertexDataType() const;
            void *getVertexCoord() const;
            bool isDeleteVertexCoord() const;

            GenericDisplayerBuilder *textureData(unsigned int, void *, bool);
            unsigned int getTextureDataType() const;
            void *getTextureCoord() const;
            bool isDeleteTextureCoord() const;

            GenericDisplayerBuilder *addTexture(Texture);
            std::vector<Texture> getTextures() const;

            std::shared_ptr<GenericDisplayer> build();

        private:
            unsigned int pNumberOfQuad;
            unsigned int pDimension;

            unsigned int vertexDataType;
            void *vertexCoord;
            bool deleteVertexCoord;

            unsigned int textureDataType;
            void *textureCoord;
            bool deleteTextureCoord;

            std::vector<Texture> textures;
    };

}

#endif

