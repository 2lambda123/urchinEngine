#include <utility>

#include <scene/ui/widget/staticbitmap/StaticBitmap.h>
#include <resources/ResourceRetriever.h>
#include <api/render/GenericRendererBuilder.h>

namespace urchin {

    StaticBitmap::StaticBitmap(Position position, Size size, std::shared_ptr<Texture> texture) :
            Widget(position, size),
            texture(std::move(texture)) {

    }

    std::shared_ptr<StaticBitmap> StaticBitmap::create(Widget* parent, Position position, Size size, const std::string& filename) {
        auto texture = ResourceRetriever::instance().getResource<Texture>(filename, {{"mipMap", "1"}});
        return Widget::create<StaticBitmap>(new StaticBitmap(position, size, std::move(texture)), parent);
    }

    //TODO should be a texture (to avoid multiple texture creation for same img)
    //TODO alternative: change constructor to use special Size !
    std::shared_ptr<StaticBitmap> StaticBitmap::create(Widget* parent, Position position, Size size, const std::shared_ptr<Image>& image) {
        auto texture = Texture::build(image->getName(), image->getWidth(), image->getHeight(), image->retrieveTextureFormat(), image->getTexels().data());
        texture->enableMipmap();
        return Widget::create<StaticBitmap>(new StaticBitmap(position, size, std::move(texture)), parent);
    }

    void StaticBitmap::createOrUpdateWidget() {
        //visual
        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(0.0f, 0.0f), Point2<float>((float)getWidth(), 0.0f), Point2<float>((float)getWidth(), (float)getHeight()),
                Point2<float>(0.0f, 0.0f), Point2<float>((float)getWidth(), (float)getHeight()), Point2<float>(0.0f, (float)getHeight())
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 0.0f), Point2<float>(1.0f, 1.0f),
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 1.0f), Point2<float>(0.0f, 1.0f)
        };
        bitmapRenderer = setupUiRenderer("static bitmap", ShapeType::TRIANGLE, true)
                ->addData(vertexCoord)
                ->addData(textureCoord)
                ->addUniformTextureReader(TextureReader::build(texture, TextureParam::build(TextureParam::EDGE_CLAMP, TextureParam::LINEAR, getTextureAnisotropy()))) //binding 2
                ->build();
    }

    WidgetType StaticBitmap::getWidgetType() const {
        return WidgetType::STATIC_BITMAP;
    }

    void StaticBitmap::prepareWidgetRendering(float, unsigned int& renderingOrder, const Matrix4<float>& projectionViewMatrix) {
        updatePositioning(bitmapRenderer.get(), projectionViewMatrix, Vector2<int>(getGlobalPositionX(), getGlobalPositionY()));
        bitmapRenderer->enableRenderer(renderingOrder);
    }

}
