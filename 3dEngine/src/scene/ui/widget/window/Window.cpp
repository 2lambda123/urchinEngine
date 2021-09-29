#include <memory>
#include <utility>
#include <UrchinCommon.h>

#include <scene/ui/widget/window/Window.h>
#include <scene/InputDeviceKey.h>
#include <graphic/render/GenericRendererBuilder.h>

namespace urchin {

    Window::Window(Position position, Size size, std::string skinName, std::string titleKey) :
            Widget(position, size),
            skinName(std::move(skinName)),
            titleKey(std::move(titleKey)),
            mousePositionX(0),
            mousePositionY(0),
            state(DEFAULT),
            title(nullptr) {

    }

    std::shared_ptr<Window> Window::create(Widget* parent, Position position, Size size, std::string skinName, std::string titleKey) {
        return Widget::create<Window>(new Window(position, size, std::move(skinName), std::move(titleKey)), parent);
    }

    void Window::createOrUpdateWidget() {
        //detach children
        detachChild(title.get());

        //skin information
        auto windowChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "window", UdaAttribute("skin", skinName));

        auto skinChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "skin", UdaAttribute(), windowChunk);
        texWindow = UISkinService::instance().createWidgetTexture(getWidth(), getHeight(), skinChunk, &widgetOutline);

        if (!titleKey.empty()) {
            auto textSkinChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "textSkin", UdaAttribute(), windowChunk);
            title = Text::createTranslatable(this, Position(0.0f, 0.0f, LengthType::PIXEL), textSkinChunk->getStringValue(), titleKey);
            title->updatePosition(Position(0.0f, -((float)widgetOutline.topWidth + (float)title->getHeight()) / 2.0f, LengthType::PIXEL));
        }

        //visual
        std::vector<Point2<float>> vertexCoord = {
                Point2<float>(0.0f, 0.0f), Point2<float>((float)getWidth(), 0.0f), Point2<float>((float)getWidth(), (float)getHeight()),
                Point2<float>(0.0f, 0.0f), Point2<float>((float)getWidth(), (float)getHeight()), Point2<float>(0.0f, (float)getHeight())
        };
        std::vector<Point2<float>> textureCoord = {
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 0.0f), Point2<float>(1.0f, 1.0f),
                Point2<float>(0.0f, 0.0f), Point2<float>(1.0f, 1.0f), Point2<float>(0.0f, 1.0f)
        };
        windowRenderer = setupUiRenderer("window", ShapeType::TRIANGLE)
                ->addData(vertexCoord)
                ->addData(textureCoord)
                ->addUniformTextureReader(TextureReader::build(texWindow, TextureParam::buildLinear())) //binding 2
                ->build();
    }

    bool Window::onKeyPressEvent(unsigned int key) {
        Rectangle<int> titleZone(Point2<int>(getGlobalPositionX(), getGlobalPositionY()),
                                 Point2<int>(getGlobalPositionX() + ((int)getWidth() - widgetOutline.rightWidth), getGlobalPositionY() + widgetOutline.topWidth));
        Rectangle<int> closeZone(Point2<int>(getGlobalPositionX() + ((int)getWidth() - widgetOutline.rightWidth), getGlobalPositionY()),
                                 Point2<int>(getGlobalPositionX() + (int)getWidth(), getGlobalPositionY() + widgetOutline.topWidth));

        if (key == (int)InputDeviceKey::MOUSE_LEFT && titleZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            mousePositionX = getMouseX() - (int)getPositionX();
            mousePositionY = getMouseY() - (int)getPositionY();

            state = MOVING;
        } else if (key == (int)InputDeviceKey::MOUSE_LEFT && closeZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            state = CLOSING;
        }

        bool propagateEvent = true;
        if (key == (int)InputDeviceKey::MOUSE_LEFT && widgetRectangle().collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            notifyObservers(this, SET_IN_FOREGROUND);
            propagateEvent = false;
        }

        return propagateEvent;
    }

    bool Window::onKeyReleaseEvent(unsigned int key) {
        Rectangle<int> closeZone(Point2<int>(getGlobalPositionX() + ((int)getWidth() - widgetOutline.rightWidth), getGlobalPositionY()),
                                 Point2<int>(getGlobalPositionX() + (int)getWidth(), getGlobalPositionY() + widgetOutline.topWidth));
        if (key == (int)InputDeviceKey::MOUSE_LEFT && state == CLOSING && closeZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            setIsVisible(false);
        }

        state = DEFAULT;

        return true;
    }

    bool Window::onMouseMoveEvent(int mouseX, int mouseY) {
        if (state == MOVING) {
            auto positionPixelX = (float)(mouseX - mousePositionX);
            auto positionLengthX = widthPixelToLength(positionPixelX, getPosition().getXType());

            auto positionPixelY = (float)(mouseY - mousePositionY);
            auto positionLengthY = heightPixelToLength(positionPixelY, getPosition().getYType());

            updatePosition(Position(positionLengthX, getPosition().getXType(),
                                    positionLengthY, getPosition().getYType(),
                                    getPosition().getRelativeTo(), getPosition().getReferencePoint()));
        }

        return true;
    }

    void Window::prepareWidgetRendering(float, unsigned int& renderingOrder) {
        updateTranslateVector(windowRenderer.get(), Vector2<int>(getGlobalPositionX(), getGlobalPositionY()));
        windowRenderer->enableRenderer(renderingOrder);
    }

}
