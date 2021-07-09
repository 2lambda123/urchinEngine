#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <UrchinCommon.h>

#include <AudioController.h>
#include <device/DeviceManager.h>
#include <sound/Sound.h>
#include <trigger/SoundTrigger.h>
#include <player/stream/StreamUpdateWorker.h>

namespace urchin {

    class SoundManager {
        public:
            SoundManager();
            ~SoundManager();

            void addSound(Sound*, SoundTrigger*);
            void removeSound(const Sound*);
            void changeSoundTrigger(const Sound*, SoundTrigger*);
            void setupSoundsVolume(Sound::SoundCategory, float);

            void setMasterVolume(float);
            float getMasterVolume() const;

            void pause();
            void unpause();

            void checkNoExceptionRaised();
            void process(const Point3<float>&);
            void process();

        private:
            void deleteAudioController(AudioController*) const;
            void adjustSoundVolume(Sound*);

            std::vector<AudioController*> audioControllers;
            std::map<Sound::SoundCategory, float> soundVolumes;

            //stream chunk updater thread
            std::unique_ptr<StreamUpdateWorker> streamUpdateWorker;
            std::unique_ptr<std::thread> streamUpdateWorkerThread;
    };

}
