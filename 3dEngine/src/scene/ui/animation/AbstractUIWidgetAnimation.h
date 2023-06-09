#pragma once

#include <scene/ui/widget/Widget.h>
#include <scene/ui/animation/AbstractUIAnimation.h>

namespace urchin {

    class AbstractUIWidgetAnimation : public AbstractUIAnimation {
        public:
            explicit AbstractUIWidgetAnimation(Widget&);
            ~AbstractUIWidgetAnimation() override = default;

            const Widget& getWidget() const;

        protected:
            void updatePosition(const Point2<float>&);
            void updateScale(const Vector2<float>&);
            void updateRotation(float);
            void updateAlphaFactor(float);

        private:
            Widget& widget;
    };

}