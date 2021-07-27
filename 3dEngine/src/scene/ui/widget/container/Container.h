#pragma once

#include <scene/ui/widget/Widget.h>
#include <scene/ui/widget/Position.h>
#include <scene/ui/widget/staticbitmap/StaticBitmap.h>
#include <scene/ui/scrollbar/Scrollbar.h>
#include <scene/ui/scrollbar/Scrollable.h>

namespace urchin {

    /**
     * Container of widgets
     */
    class Container : public Widget, public Scrollable {
        public:
            static std::shared_ptr<Container> newContainer(Widget*, Position);
            static std::shared_ptr<Container> newContainer(Widget*, Position, Size);

            void onResize(unsigned int, unsigned int) override;

            void addChild(const std::shared_ptr<Widget>&) override;
            void detachChild(Widget*) override;
            void detachChildren() override;
            void resetChildren();

            void enableScissor(bool);
            bool isScissorEnabled() const;

            void enableScrollbar(bool, const std::string& = "");
            bool isScrollbarEnabled() const;
            int getScrollShiftY() const override;

        protected:
            Container(Position, Size);

            void createOrUpdateWidget() override;
            bool onKeyPressEvent(unsigned int) override;
            bool onKeyReleaseEvent(unsigned int) override;
            bool onMouseMoveEvent(int, int) override;
            bool onScrollEvent(double) override;

            void prepareWidgetRendering(float) override;

        private:
            std::shared_ptr<Texture> loadTexture(const UdaChunk*, const std::string&) const;

            bool scissorEnabled;
            std::unique_ptr<Scrollbar> scrollbar;
    };

}
