#pragma once

#include <string>
#include <list>
#include <utility>

#include <UrchinCommon.h>
#include <Urchin3dEngine.h>
#include <UrchinSoundEngine.h>
#include <UrchinAIEngine.h>
#include <map/save/LoadMapCallback.h>
#include <model/ObjectEntity.h>
#include <model/LightEntity.h>
#include <model/TerrainEntity.h>
#include <model/WaterEntity.h>
#include <model/SkyEntity.h>
#include <model/SoundEntity.h>

namespace urchin {

    class Map {
        public:
            Map(Renderer3d*, PhysicsWorld&, SoundEnvironment&, AIEnvironment&);

            Renderer3d* getRenderer3d() const;
            PhysicsWorld& getPhysicsWorld() const;
            SoundEnvironment& getSoundEnvironment() const;
            AIEnvironment& getAIEnvironment() const;

            void setRelativeWorkingDirectory(std::string);
            const std::string& getRelativeWorkingDirectory() const;

            const std::list<std::unique_ptr<ObjectEntity>>& getObjectEntities() const;
            ObjectEntity& getObjectEntity(std::string_view) const;
            void findObjectEntitiesByTag(std::string_view, std::vector<ObjectEntity*>&) const;
            ObjectEntity* findObjectEntityByTag(std::string_view) const;
            ObjectEntity& addObjectEntity(std::unique_ptr<ObjectEntity>);
            void removeObjectEntity(ObjectEntity&);
            bool moveUpObjectEntity(ObjectEntity&);
            bool moveDownObjectEntity(ObjectEntity&);
            template<class Compare> void sortObjectEntity(Compare);

            const std::list<std::unique_ptr<LightEntity>>& getLightEntities() const;
            LightEntity& getLightEntity(std::string_view) const;
            LightEntity& addLightEntity(std::unique_ptr<LightEntity>);
            void removeLightEntity(LightEntity&);

            const std::list<std::unique_ptr<TerrainEntity>>& getTerrainEntities() const;
            TerrainEntity& getTerrainEntity(std::string_view) const;
            TerrainEntity& addTerrainEntity(std::unique_ptr<TerrainEntity>);
            void removeTerrainEntity(TerrainEntity&);

            const std::list<std::unique_ptr<WaterEntity>>& getWaterEntities() const;
            WaterEntity& getWaterEntity(std::string_view) const;
            WaterEntity& addWaterEntity(std::unique_ptr<WaterEntity>);
            void removeWaterEntity(WaterEntity&);

            const SkyEntity& getSkyEntity() const;
            void setSkyEntity(std::unique_ptr<SkyEntity>);

            const std::list<std::unique_ptr<SoundEntity>>& getSoundEntities() const;
            SoundEntity& getSoundEntity(std::string_view) const;
            SoundEntity& addSoundEntity(std::unique_ptr<SoundEntity>);
            void removeSoundEntity(SoundEntity&);

            void refresh() const;
            void pause();
            void unpause();

        private:
            Renderer3d* renderer3d;
            PhysicsWorld& physicsWorld;
            SoundEnvironment& soundEnvironment;
            AIEnvironment& aiEnvironment;

            std::string relativeWorkingDirectory;

            TagHolder objectEntitiesTagHolder;
            mutable std::vector<ObjectEntity*> tmpObjectEntities;
            std::list<std::unique_ptr<ObjectEntity>> objectEntities;
            std::list<std::unique_ptr<LightEntity>> lightEntities;
            std::list<std::unique_ptr<TerrainEntity>> terrainEntities;
            std::list<std::unique_ptr<WaterEntity>> waterEntities;
            std::unique_ptr<SkyEntity> skyEntity;
            std::list<std::unique_ptr<SoundEntity>> soundEntities;
    };

    #include "Map.inl"

}
