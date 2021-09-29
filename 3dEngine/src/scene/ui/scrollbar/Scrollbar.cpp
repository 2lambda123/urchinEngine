#include <utility>

#include <scene/ui/scrollbar/Scrollbar.h>
#include <scene/ui/scrollbar/Scrollable.h>
#include <resources/ResourceRetriever.h>
#include <scene/InputDeviceKey.h>

namespace urchin {

    Scrollbar::Scrollbar(Widget& scrollableWidget, std::string skinName) :
            SCROLL_SPEED(ConfigService::instance().getFloatValue("ui.scrollSpeed")),
            scrollableWidget(scrollableWidget),
            skinName(std::move(skinName)),
            visibleHeight(0),
            contentHeight(0),
            mouseX(0),
            mouseY(0),
            state(DEFAULT),
            scrollPercentage(0.0f),
            shiftPixelPositionY(0) {

    }

    void Scrollbar::initializeOrUpdate() {
        //detach children
        if (scrollbarCursor && scrollbarCursor->getParent() == &scrollableWidget) {
            scrollableWidget.detachChild(scrollbarCursor.get());
            scrollableWidget.detachChild(scrollbarLine.get());
        }

        //skin information
        auto scrollbarChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "scrollbar", UdaAttribute("skin", skinName));

        auto cursorImageChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "imageCursor", UdaAttribute(), scrollbarChunk);
        std::string cursorImageFilename = cursorImageChunk->getStringValue();
        LengthType scrollbarWidthType;
        float scrollbarWidth = UISkinService::instance().loadLength(scrollbarChunk, "width", scrollbarWidthType);
        auto imageCursor = loadTexture(scrollbarChunk, "imageCursor");
        auto cursorImageRatio = (float)imageCursor->getHeight() / (float)imageCursor->getWidth();
        auto cursorWidthInPixel = scrollableWidget.widthLengthToPixel(scrollbarWidth, scrollbarWidthType, [](){return 0.0f;});

        auto lineImageChunk = UISkinService::instance().getSkinReader().getUniqueChunk(true, "imageLine", UdaAttribute(), scrollbarChunk);
        std::string lineImageFilename = lineImageChunk->getStringValue();
        auto imageLine = loadTexture(scrollbarChunk, "imageLine");
        if (imageCursor->getWidth() != imageLine->getWidth()) {
            throw std::runtime_error("Cursor and line images must have the same width");
        }

        scrollbarLine = StaticBitmap::create(&scrollableWidget, Position((float)scrollableWidget.getWidth() - (float)cursorWidthInPixel, 0.0f, LengthType::PIXEL),
                                             Size(scrollbarWidth, scrollbarWidthType, (float)scrollableWidget.getHeight(), LengthType::PIXEL), lineImageFilename);
        scrollbarCursor = StaticBitmap::create(&scrollableWidget, Position((float)scrollableWidget.getWidth() - (float)cursorWidthInPixel, 0.0f, LengthType::PIXEL),
                                               Size(scrollbarWidth, scrollbarWidthType, cursorImageRatio, LengthType::RATIO_TO_WIDTH), cursorImageFilename);

        //update scrollbar
        onScrollableWidgetsUpdated();
    }

    void Scrollbar::onScrollableWidgetsUpdated() {
        //compute values
        float minChildPositionY = 100000.0f;
        float maxChildPositionY = 0.0f;
        for (auto& contentChild : getContentChildren()) {
            auto childMinPositionY = (float)contentChild->getGlobalPositionY();
            if (childMinPositionY < minChildPositionY) {
                minChildPositionY = childMinPositionY;
            }

            auto childMaxPositionY = (float)contentChild->getGlobalPositionY() + (float)contentChild->getHeight();
            if (childMaxPositionY > maxChildPositionY) {
                maxChildPositionY = childMaxPositionY;
            }
        }
        contentHeight = maxChildPositionY - minChildPositionY;
        visibleHeight = (float)scrollableWidget.getHeight();

        updateScrollingPosition();
    }

    bool Scrollbar::onKeyPressEvent(unsigned int key) {
        if (key == (int)InputDeviceKey::MOUSE_LEFT) {
            if (scrollbarCursor->widgetRectangle().collideWithPoint(Point2<int>(mouseX, mouseY))) {
                state = CURSOR_SELECTED;
            } else if (scrollbarLine->widgetRectangle().collideWithPoint(Point2<int>(mouseX, mouseY))) {
                updateScrollingPosition(mouseY);
                state = CURSOR_SELECTED;
            }
        }
        return true;
    }

    bool Scrollbar::onKeyReleaseEvent(unsigned int key) {
        if (key == (int)InputDeviceKey::MOUSE_LEFT) {
            state = DEFAULT;
        }
        return true;
    }

    bool Scrollbar::onMouseMoveEvent(int mouseX, int mouseY) {
        this->mouseX = mouseX;
        this->mouseY = mouseY;

        if (state == CURSOR_SELECTED) {
            updateScrollingPosition(mouseY);
            return false;
        }
        return true;
    }

    bool Scrollbar::onScrollEvent(double offsetY) {
        if (scrollableWidget.getWidgetState() == Widget::WidgetStates::FOCUS && isScrollbarRequired()) {
            float scrollMoveSpeedFactor = visibleHeight / (contentHeight - visibleHeight);
            float deltaScroll = (float) offsetY * SCROLL_SPEED * scrollMoveSpeedFactor;
            scrollPercentage -= deltaScroll;
            scrollPercentage = MathFunction::clamp(scrollPercentage, 0.0f, 1.0f);

            updateScrollingPosition();
            return false;
        }
        return true;
    }

    bool Scrollbar::isScrollbarWidget(Widget* widget) const {
        return widget == scrollbarCursor.get() || widget == scrollbarLine.get();
    }

    bool Scrollbar::isScrollbarRequired() const {
        return contentHeight > visibleHeight;
    }

    int Scrollbar::getScrollShiftY() const {
        return shiftPixelPositionY;
    }

    std::shared_ptr<Texture> Scrollbar::loadTexture(const UdaChunk* scrollbarChunk, const std::string& chunkName) const {
        auto imageElem = UISkinService::instance().getSkinReader().getUniqueChunk(true, chunkName, UdaAttribute(), scrollbarChunk);
        auto img = ResourceRetriever::instance().getResource<Image>(imageElem->getStringValue());
        return img->createTexture(false);
    }

    void Scrollbar::updateScrollingPosition(int positionY) {
        auto halfCursorHeight = (float)scrollbarCursor->getHeight() / 2.0f;
        auto minPositionY = (float)scrollbarLine->getGlobalPositionY() + halfCursorHeight;
        auto maxPositionY = (float)scrollbarLine->getGlobalPositionY() + (float)scrollbarLine->getHeight() - halfCursorHeight;

        scrollPercentage = ((float)positionY - minPositionY) / (maxPositionY - minPositionY);
        scrollPercentage = MathFunction::clamp(scrollPercentage, 0.0f, 1.0f);

        updateScrollingPosition();
    }

    void Scrollbar::updateScrollingPosition() {
        if (scrollbarLine && scrollbarCursor) {
            scrollbarLine->setIsVisible(isScrollbarRequired());
            scrollbarCursor->setIsVisible(isScrollbarRequired());

            if (isScrollbarRequired()) {
                updateCursorPosition();
                computeShiftPositionY();
            } else if (!MathFunction::isZero(scrollPercentage)) {
                scrollPercentage = 0.0f;
                computeShiftPositionY();
            }
            dynamic_cast<Scrollable&>(this->scrollableWidget).onScrollableContentUpdated();
        }
    }

    void Scrollbar::updateCursorPosition() {
        float cursorPositionX = scrollbarCursor->getPosition().getX();
        LengthType cursorPositionXType = scrollbarCursor->getPosition().getXType();

        auto cursorMaxPositionY = (float)scrollableWidget.getHeight() - (float)scrollbarCursor->getHeight();
        float cursorPositionY = cursorMaxPositionY * scrollPercentage;

        scrollbarCursor->updatePosition(Position(cursorPositionX, cursorPositionXType, cursorPositionY, LengthType::PIXEL));
    }

    void Scrollbar::computeShiftPositionY() {
        shiftPixelPositionY = -(int)((contentHeight - visibleHeight) * scrollPercentage);

        //compensate the shift applied on all children (including scrollbarLine & scrollbarCursor)
        scrollbarLine->updatePosition(Position(scrollbarLine->getPosition().getX(), 0.0f - (float)shiftPixelPositionY, LengthType::PIXEL));
        scrollbarCursor->updatePosition(Position(scrollbarCursor->getPosition().getX(), scrollbarCursor->getPosition().getY() - (float)shiftPixelPositionY, LengthType::PIXEL));
    }

    std::vector<Widget*> Scrollbar::getContentChildren() const {
        std::vector<Widget*> contentChildren;
        contentChildren.reserve(scrollableWidget.getChildren().size());

        for (auto& child : scrollableWidget.getChildren()) {
            if (!isScrollbarWidget(child.get())){
                contentChildren.push_back(child.get());
            }
        }
        return contentChildren;
    }

}