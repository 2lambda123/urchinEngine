#ifndef URCHINENGINE_FRAMEBUFFERSIZERETRIEVER_H
#define URCHINENGINE_FRAMEBUFFERSIZERETRIEVER_H

namespace urchin {

    class FramebufferSizeRetriever {
        public:
            /**
             * @param widthInPixel [out] return width in pixel (<> screen coordinate)
             * @param heightInPixel [out] return height in pixel (<> screen coordinate)
             */
            virtual void getFramebufferSizeInPixel(int& widthInPixel, int& heightInPixel) const = 0;
    };

}

#endif
