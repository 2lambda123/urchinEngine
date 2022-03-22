#pragma once

#include <UrchinCommon.h>

namespace urchin {

    class AudioDevice : public Singleton<AudioDevice> {
        public:
            friend class Singleton<AudioDevice>;

            ~AudioDevice() override;

            void enable();

        private:
            AudioDevice();

            ALCdevice* device;
            ALCcontext* context;
    };

}
