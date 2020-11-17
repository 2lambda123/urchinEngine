#include <utility>

#include "scene/GUI/widget/staticbitmap/StaticBitmap.h"
#include "resources/MediaManager.h"
#include "graphic/render/GenericRendererBuilder.h"

namespace urchin {

    StaticBitmap::StaticBitmap(Position position, Size size, std::string filename) :
        Widget(position, size),
        filename(std::move(filename)),
        tex(nullptr) {
        StaticBitmap::createOrUpdateWidget();
    }

    StaticBitmap::~StaticBitmap() {
        tex->release();
    }

    void StaticBitmap::createOrUpdateWidget() {
        //loads the texture
        if (tex) {
            tex->release();
        }
        tex = MediaManager::instance()->getMedia<Image>(filename);
        tex->toTexture(false, false, false);

        //visual
        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(0.0f, 0.0f),
                Point2<float>(getWidth(), 0.0f),
                Point2<float>(getWidth(), getHeight()),
                Point2<float>(0.0f, getHeight())
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 0.0f),
                Point2<float>(1.0f, 0.0f),
                Point2<float>(1.0f, 1.0f),
                Point2<float>(0.0f, 1.0f)
        };
        bitmapRenderer = std::make_unique<GenericRendererBuilder>(ShapeType::RECTANGLE)
                ->addData(&vertexCoord)
                ->addData(&textureCoord)
                ->addTexture(Texture::build(tex->getTextureID()))
                ->enableTransparency()
                ->build();
    }

    void StaticBitmap::display(const ShaderVar &translateDistanceShaderVar, float dt) {
        bitmapRenderer->draw();

        Widget::display(translateDistanceShaderVar, dt);
    }

}
