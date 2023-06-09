#pragma once

#include <list>
#include <UrchinCommon.h>
#include <UrchinAggregation.h>
#include <Urchin3dEngine.h>

#include <controller/AbstractController.h>

namespace urchin {

    class TerrainController : public AbstractController {
        public:
            TerrainController();

            std::list<const TerrainEntity*> getTerrainEntities() const;
            void addTerrainEntity(std::unique_ptr<TerrainEntity>);
            void removeTerrainEntity(const TerrainEntity&);

            const TerrainEntity& updateTerrainGeneralProperties(const TerrainEntity&, const Point3<float>&, float);
            const TerrainEntity& updateTerrainMesh(const TerrainEntity&, float, float);
            const TerrainEntity& updateTerrainMaterial(const TerrainEntity&, float, float, std::string, const std::vector<std::string>&);
            const TerrainEntity& updateTerrainGrass(const TerrainEntity&, std::string, std::string, unsigned int, float, float, float, const Vector3<float>&, float);

        private:
            TerrainEntity& findTerrainEntity(const TerrainEntity&);
    };

}
