#include <map/save/sound/SoundEntityReaderWriter.h>
#include <map/save/sound/SoundReaderWriter.h>
#include <map/save/sound/SoundTriggerReaderWriter.h>

namespace urchin {

    std::unique_ptr<SoundEntity> SoundEntityReaderWriter::load(const UdaChunk* soundEntityChunk, const UdaParser& udaParser) {
        auto soundEntity = std::make_unique<SoundEntity>();

        soundEntity->setName(soundEntityChunk->getAttributeValue(NAME_ATTR));

        auto soundChunk = udaParser.getUniqueChunk(true, SOUND_TAG, UdaAttribute(), soundEntityChunk);
        auto soundTriggerChunk = udaParser.getUniqueChunk(true, SOUND_TRIGGER_TAG, UdaAttribute(), soundEntityChunk);
        soundEntity->setSoundElements(SoundReaderWriter::load(soundChunk, udaParser), SoundTriggerReaderWriter::load(soundTriggerChunk, udaParser));

        return soundEntity;
    }

    void SoundEntityReaderWriter::write(UdaChunk& soundEntityChunk, const SoundEntity& soundEntity, UdaWriter& udaWriter) {
        soundEntityChunk.addAttribute(UdaAttribute(NAME_ATTR, soundEntity.getName()));

        auto& soundChunk = udaWriter.createChunk(SOUND_TAG, UdaAttribute(), &soundEntityChunk);
        auto& soundTriggerChunk = udaWriter.createChunk(SOUND_TRIGGER_TAG, UdaAttribute(), &soundEntityChunk);
        SoundReaderWriter::write(soundChunk, *soundEntity.getSound(), udaWriter);
        SoundTriggerReaderWriter::write(soundTriggerChunk, *soundEntity.getSoundTrigger(), udaWriter);
    }

}