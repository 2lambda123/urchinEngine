#ifndef URCHINENGINE_WINDOW_H
#define URCHINENGINE_WINDOW_H

#include <string>
#include <memory>

#include "scene/UI/widget/Widget.h"
#include "scene/UI/widget/Position.h"
#include "scene/UI/widget/Size.h"
#include "scene/UI/widget/text/Text.h"
#include "resources/image/Image.h"
#include "graphic/render/GenericRenderer.h"

namespace urchin {

    class Window : public Widget {
        public:
            Window(Widget*, Position, Size, std::string, std::string);
            Window(Position, Size, std::string, std::string);

        protected:
            void createOrUpdateWidget() override;
            void displayWidget(float) override;

        private:
            bool onKeyPressEvent(unsigned int) override;
            bool onKeyReleaseEvent(unsigned int) override;
            bool onMouseMoveEvent(int, int) override;

            //properties
            const std::string nameSkin;
            const std::string stringTitle;

            //state
            int mousePositionX, mousePositionY;
            enum windowStates {
                DEFAULT,
                MOVING,
                CLOSING
            } state;

            //visual
            Text* title;
            std::shared_ptr<Texture> texWindow;
            std::unique_ptr<GenericRenderer> windowRenderer;
    };

}

#endif
