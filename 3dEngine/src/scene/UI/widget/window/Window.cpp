#include <memory>
#include <utility>
#include "UrchinCommon.h"

#include "scene/UI/widget/window/Window.h"
#include "scene/InputDeviceKey.h"
#include "graphic/render/GenericRendererBuilder.h"

namespace urchin {

    Window::Window(Position position, Size size, std::string nameSkin, std::string stringTitle) :
            Widget(position, size),
            nameSkin(std::move(nameSkin)),
            stringTitle(std::move(stringTitle)),
            mousePositionX(0),
            mousePositionY(0),
            state(DEFAULT),
            title(nullptr),
            widgetOutline(new WidgetOutline()) {
        Window::createOrUpdateWidget();
    }

    Window::~Window() {
        delete widgetOutline;
    }

    void Window::createOrUpdateWidget() {
        //load the skin
        std::shared_ptr<XmlChunk> windowChunk = UISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "window", XmlAttribute("nameSkin", nameSkin));

        //creates the texture
        std::shared_ptr<XmlChunk> skinChunk = UISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "skin", XmlAttribute(), windowChunk);
        texWindow = UISkinService::instance()->createWidgetTexture(getWidth(), getHeight(), skinChunk, widgetOutline);

        //creates font for title
        if (!stringTitle.empty()) {
            std::shared_ptr<XmlChunk> textSkinChunk = UISkinService::instance()->getXmlSkin()->getUniqueChunk(true, "textSkin", XmlAttribute(), windowChunk);
            Widget::removeChild(title);
            title = new Text(Position(0, 0, Position::PIXEL), textSkinChunk->getStringValue(), stringTitle);
            title->setPosition(Position((float)(widgetOutline->leftWidth) + 1.0f, (float)(widgetOutline->topWidth - title->getHeight()) / 2.0f, Position::PIXEL));
            Widget::addChild(title);
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
        windowRenderer = std::make_unique<GenericRendererBuilder>(ShapeType::TRIANGLE)
                ->addData(&vertexCoord)
                ->addData(&textureCoord)
                ->addTexture(TextureReader::build(texWindow, TextureParam::buildNearest()))
                ->build();
    }

    void Window::addChild(Widget* child) {
        Position childPosition((float)(child->getPositionX() + (int)widgetOutline->leftWidth), (float)(child->getPositionY() + (int)widgetOutline->topWidth), Position::PIXEL);
        child->setPosition(childPosition);
        Widget::addChild(child);
    }

    bool Window::onKeyPressEvent(unsigned int key) {
        Rectangle<int> titleZone(Point2<int>(getGlobalPositionX(), getGlobalPositionY()),
                Point2<int>(getGlobalPositionX() + ((int)getWidth() - (int)widgetOutline->rightWidth), getGlobalPositionY() + (int)widgetOutline->topWidth));
        Rectangle<int> closeZone(Point2<int>(getGlobalPositionX()+((int)getWidth() - (int)widgetOutline->rightWidth), getGlobalPositionY()),
                Point2<int>(getGlobalPositionX() + (int)getWidth(), getGlobalPositionY() + (int)widgetOutline->topWidth));

        if (key == InputDeviceKey::MOUSE_LEFT && titleZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            mousePositionX = getMouseX() - getPositionX();
            mousePositionY = getMouseY() - getPositionY();

            state = MOVING;
        } else if (key == InputDeviceKey::MOUSE_LEFT && closeZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            state = CLOSING;
        }

        Rectangle<int> widgetRectangle(Point2<int>(getGlobalPositionX(), getGlobalPositionY()),
                Point2<int>(getGlobalPositionX() + (int)getWidth(), getGlobalPositionY() + (int)getHeight()));
        bool propagateEvent = true;
        if (key == InputDeviceKey::MOUSE_LEFT && widgetRectangle.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            notifyObservers(this, SET_IN_FOREGROUND);
            propagateEvent = false;
        }

        return propagateEvent;
    }

    bool Window::onKeyReleaseEvent(unsigned int key) {
        Rectangle<int> closeZone(Point2<int>(getGlobalPositionX() + ((int)getWidth() - (int)widgetOutline->rightWidth), getGlobalPositionY()),
                Point2<int>(getGlobalPositionX() + (int)getWidth(), getGlobalPositionY() + (int)widgetOutline->topWidth));
        if (key == InputDeviceKey::MOUSE_LEFT && state == CLOSING && closeZone.collideWithPoint(Point2<int>(getMouseX(), getMouseY()))) {
            setIsVisible(false);
        }

        state = DEFAULT;

        return true;
    }

    bool Window::onMouseMoveEvent(int mouseX, int mouseY) {
        if (state == MOVING) {
            auto positionX = (float)(mouseX - mousePositionX);
            if (getPosition().getPositionTypeX() == Position::PERCENTAGE) {
                positionX /= (float)getSceneWidth();
            }

            auto positionY = (float)(mouseY - mousePositionY);
            if (getPosition().getPositionTypeY() == Position::PERCENTAGE) {
                positionY /= (float)getSceneHeight();
            }

            setPosition(Position(positionX, getPosition().getPositionTypeX(), positionY, getPosition().getPositionTypeY()));
        }

        return true;
    }

    void Window::display(const RenderTarget* renderTarget, const ShaderVar& translateDistanceShaderVar, float dt) {
        renderTarget->display(windowRenderer);

        Widget::display(renderTarget, translateDistanceShaderVar, dt);
    }

}