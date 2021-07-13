#include <stdexcept>

#include "SceneSound.h"
#include <resources/sound/SoundReaderWriter.h>
#include <resources/sound/SoundTriggerReaderWriter.h>

namespace urchin {

    SceneSound::SceneSound() :
            soundManager(nullptr) {

    }

    SceneSound::~SceneSound() {
        if (soundManager) {
            soundManager->removeSound(*sound);
        }
    }

    void SceneSound::setSoundManager(SoundManager* soundManager) {
        this->soundManager = soundManager;

        if (soundManager) {
            soundManager->addSound(sound, soundTrigger);
        }
    }

    void SceneSound::loadFrom(const DataChunk* chunk, const DataParser& dataParser) {
        this->name = chunk->getAttributeValue(NAME_ATTR);

        auto soundChunk = dataParser.getUniqueChunk(true, SOUND_TAG, UdaAttribute(), chunk);
        auto soundTriggerChunk = dataParser.getUniqueChunk(true, SOUND_TRIGGER_TAG, UdaAttribute(), chunk);

        setSoundElements(SoundReaderWriter::loadFrom(soundChunk, dataParser), SoundTriggerReaderWriter::loadFrom(soundTriggerChunk, dataParser));
    }

    void SceneSound::writeOn(DataChunk& chunk, UdaWriter& udaWriter) const {
        chunk.addAttribute(UdaAttribute(NAME_ATTR, this->name));

        auto& soundChunk = udaWriter.createChunk(SOUND_TAG, UdaAttribute(), &chunk);
        auto& soundTriggerChunk = udaWriter.createChunk(SOUND_TRIGGER_TAG, UdaAttribute(), &chunk);

        SoundReaderWriter::writeOn(soundChunk, *sound, udaWriter);
        SoundTriggerReaderWriter::writeOn(soundTriggerChunk, *soundTrigger, udaWriter);
    }

    std::string SceneSound::getName() const {
        return name;
    }

    void SceneSound::setName(const std::string& name) {
        this->name = name;
    }

    Sound* SceneSound::getSound() const {
        return sound.get();
    }

    SoundTrigger* SceneSound::getSoundTrigger() const {
        return soundTrigger.get();
    }

    void SceneSound::setSoundElements(const std::shared_ptr<Sound>& sound, const std::shared_ptr<SoundTrigger>& soundTrigger) {
        if (!sound) {
            throw std::invalid_argument("Cannot set a null sound on scene sound.");
        } else if (!soundTrigger) {
            throw std::invalid_argument("Cannot set a null sound trigger on scene sound.");
        }

        if (soundManager) {
            soundManager->removeSound(*this->sound);
            soundManager->addSound(sound, soundTrigger);
        }

        this->sound = sound;
        this->soundTrigger = soundTrigger;
    }

    void SceneSound::changeSoundTrigger(const std::shared_ptr<SoundTrigger>& newSoundTrigger) {
        if (!sound) {
            throw std::invalid_argument("Cannot change sound trigger without having a sound on scene sound.");
        }
        if (!soundTrigger) {
            throw std::invalid_argument("Cannot set a null sound trigger on scene sound.");
        }

        if (soundManager) {
            soundManager->changeSoundTrigger(*sound, newSoundTrigger);
        }

        this->soundTrigger = newSoundTrigger;
    }
}
