#include <utility>

#include <sound/spatial/SpatialSound.h>

#define DEFAULT_INAUDIBLE_DISTANCE 10.0

namespace urchin {

    /**
     * @param inaudibleDistance Define distance at which the sound become inaudible
     */
    SpatialSound::SpatialSound(std::string filename, SoundCategory category, const Point3<float>& position, float inaudibleDistance) :
            Sound(std::move(filename), category),
            position(position),
            inaudibleDistance(inaudibleDistance) {
        initializeSource();
    }

    SpatialSound::SpatialSound(std::string filename, SoundCategory category, const Point3<float>& position) :
            SpatialSound(std::move(filename), category, position, DEFAULT_INAUDIBLE_DISTANCE) {

    }

    void SpatialSound::initializeSource() {
        alSourcei(getSourceId(), AL_SOURCE_RELATIVE, false);
        alSource3f(getSourceId(), AL_POSITION, position.X, position.Y, position.Z);

        alSourcef(getSourceId(), AL_MAX_DISTANCE, inaudibleDistance);
        alSourcef(getSourceId(), AL_REFERENCE_DISTANCE, 0.0);
        alSourcef(getSourceId(), AL_ROLLOFF_FACTOR, 1.0);
    }

    Sound::SoundType SpatialSound::getSoundType() const {
        return Sound::SPATIAL;
    }

    void SpatialSound::setPosition(const Point3<float>& position) {
        this->position = position;
        alSource3f(getSourceId(), AL_POSITION, position.X, position.Y, position.Z);
    }

    Point3<float> SpatialSound::getPosition() const {
        return position;
    }

    /**
     * @param inaudibleDistance Inaudible distance of the sound
     */
    void SpatialSound::setInaudibleDistance(float inaudibleDistance) {
        this->inaudibleDistance = inaudibleDistance;
        alSourcef(getSourceId(), AL_MAX_DISTANCE, inaudibleDistance);
    }

    /**
     * @return Distance at which the sound become inaudible
     */
    float SpatialSound::getInaudibleDistance() const {
        return inaudibleDistance;
    }

}