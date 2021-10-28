#pragma once

#include <scene/ui/widget/Widget.h>
#include <scene/ui/widget/Position.h>
#include <scene/ui/widget/Size.h>

namespace urchin {

    class CheckBox : public Widget {
        public:
            static std::shared_ptr<CheckBox> create(Widget*, Position, Size, std::string);

            void setChecked(bool);
            bool isChecked() const;

        protected:
            void createOrUpdateWidget() override;
            void prepareWidgetRendering(float, unsigned int&, const Matrix4<float>&) override;

        private:
            CheckBox(Position, Size, std::string);

            std::shared_ptr<Texture> loadTexture(const UdaChunk*, const std::string&) const;
            void refreshTexture();
            bool onKeyReleaseEvent(unsigned int) override;
            void switchValue();

            //properties
            const std::string skinName;

            //state
            bool bIsChecked;

            //visual
            std::shared_ptr<Texture> texChecked, texUnchecked;
            std::unique_ptr<GenericRenderer> checkBoxRenderer;
    };

}
