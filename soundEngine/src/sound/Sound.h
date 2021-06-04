#pragma once

#include <string>
#include <AL/al.h>

namespace urchin {

    class Sound {
        public:
            enum SoundType {
                POINT,
                AMBIENT
            };

            explicit Sound(std::string );
            virtual ~Sound();

            virtual SoundType getSoundType() const = 0;
            ALuint getSourceId() const;
            const std::string& getFilename() const;

            bool isStopped() const;
            bool isPaused() const;
            bool isPlaying() const;

            void setVolume(float);
            void setVolumeChange(float);
            float getVolume() const;

        private:
            ALuint sourceId;

            std::string filename;
            float volume;
    };

}
