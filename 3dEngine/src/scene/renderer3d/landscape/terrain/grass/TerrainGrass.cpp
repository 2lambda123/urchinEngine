#include <random>
#include <stack>
#include <cassert>
#include <thread>
#include <functional>

#include <scene/renderer3d/landscape/terrain/grass/TerrainGrass.h>
#include <resources/ResourceRetriever.h>
#include <graphics/render/shader/ShaderBuilder.h>
#include <graphics/render/GenericRendererBuilder.h>

namespace urchin {

    TerrainGrass::TerrainGrass(const std::string& grassTextureFilename) :
            grassPatchSize(ConfigService::instance().getFloatValue("terrain.grassPatchSize")),
            grassQuadtreeDepth(ConfigService::instance().getUnsignedIntValue("terrain.grassQuadtreeDepth")),
            bIsInitialized(false),
            renderTarget(nullptr),
            positioningData({}),
            grassProperties({}),
            terrainPositioningData({}),
            ambient(0.5f),
            mesh(nullptr),
            grassQuantity(0.0f) {
        float grassAlphaTest = ConfigService::instance().getFloatValue("terrain.grassAlphaTest");
        std::vector<std::size_t> variablesSize = {sizeof(grassAlphaTest)};
        auto shaderConstants = std::make_unique<ShaderConstants>(variablesSize, &grassAlphaTest);
        terrainGrassShader = ShaderBuilder::createShader("terrainGrass.vert.spv", "terrainGrass.geom.spv", "terrainGrass.frag.spv", std::move(shaderConstants));

        setGrassTexture(grassTextureFilename);
        setMaskTexture("");
        setNumGrassInTexture(1);
        setGrassDisplayDistance(100.0f);
        setGrassHeight(1.0f);
        setGrassLength(1.0f);
        setGrassQuantity(0.1f);
        setWindDirection(Vector3<float>(1.0f, 0.0f, 0.0f));
        setWindStrength(1.0f);
    }

    void TerrainGrass::initialize(RenderTarget& renderTarget) {
        assert(!bIsInitialized);

        this->renderTarget = &renderTarget;

        bIsInitialized = true;
    }

    bool TerrainGrass::isInitialized() const {
        return bIsInitialized;
    }

    void TerrainGrass::refreshWith(const TerrainMesh* mesh, const Point3<float>& terrainPosition) {
        assert(bIsInitialized);
        assert(mesh);
        generateGrass(mesh, terrainPosition);

        terrainPositioningData.minPoint = mesh->getVertices()[0];
        terrainPositioningData.maxPoint = mesh->getVertices()[mesh->getXSize() * mesh->getZSize() - 1];

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(2, &terrainPositioningData);
        }
    }

    void TerrainGrass::refreshWith(float ambient) {
        assert(bIsInitialized);
        this->ambient = ambient;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(3, &ambient);
        }
    }

    void TerrainGrass::generateGrass(const TerrainMesh* mesh, const Point3<float>& terrainPosition) {
        const unsigned int NUM_THREADS = std::max(2u, std::thread::hardware_concurrency());

        if (mesh) {
            this->mesh = mesh;
            this->terrainPosition = terrainPosition;

            unsigned int seed = 0; //no need to generate different random numbers at each start
            std::default_random_engine generator(seed);
            std::uniform_real_distribution<float> distribution(-GRASS_POSITION_RANDOM_PERCENTAGE / grassQuantity, GRASS_POSITION_RANDOM_PERCENTAGE / grassQuantity);

            auto grassXQuantity = MathFunction::roundToUInt(mesh->getXZScale() * (float)mesh->getXSize() * grassQuantity);
            auto grassZQuantity = MathFunction::roundToUInt(mesh->getXZScale() * (float)mesh->getZSize() * grassQuantity);

            auto patchQuantityX = MathFunction::roundToUInt(mesh->getXZScale() * (float)mesh->getXSize() / grassPatchSize);
            auto patchQuantityZ = MathFunction::roundToUInt(mesh->getXZScale() * (float)mesh->getZSize() / grassPatchSize);
            float adjustedPatchSizeX = mesh->getXZScale() * (float)mesh->getXSize() / (float)patchQuantityX;
            float adjustedPatchSizeZ = mesh->getXZScale() * (float)mesh->getZSize() / (float)patchQuantityZ;

            std::vector<std::unique_ptr<TerrainGrassQuadtree>> leafGrassPatches;
            leafGrassPatches.reserve(patchQuantityX * patchQuantityZ);
            for (unsigned int i = 0; i < patchQuantityX * patchQuantityZ; ++i) {
                leafGrassPatches.push_back(std::make_unique<TerrainGrassQuadtree>());
            }

            float startX = mesh->getVertices()[0].X;
            float startZ = mesh->getVertices()[0].Z;

            std::vector<std::jthread> threads(NUM_THREADS);
            for (unsigned int threadI = 0; threadI < NUM_THREADS; threadI++) {
                unsigned int beginX = threadI * grassXQuantity / NUM_THREADS;
                unsigned int endX = (threadI + 1) == NUM_THREADS ? grassXQuantity : (threadI + 1) * grassXQuantity / NUM_THREADS;

                threads[threadI] = std::jthread([&, beginX, endX]() {
                    for (unsigned int xIndex = beginX; xIndex < endX; ++xIndex) {
                        const float xFixedValue = startX + (float)xIndex / grassQuantity;

                        for (unsigned int zIndex = 0; zIndex < grassZQuantity; ++zIndex) {
                            float xValue = xFixedValue + distribution(generator);
                            float zValue = (startZ + (float)zIndex / grassQuantity) + distribution(generator);
                            unsigned int vertexIndex = retrieveVertexIndex(Point2<float>(xValue, zValue));
                            float yValue = (mesh->getVertices()[vertexIndex] + terrainPosition).Y;

                            Point3 globalGrassVertex(xValue + terrainPosition.X, yValue, zValue + terrainPosition.Z);
                            Vector3<float> grassNormal = (mesh->getNormals()[vertexIndex] / 2.0f) + Vector3<float>(0.5f, 0.5f, 0.5f);

                            unsigned int patchXIndex = std::min((unsigned int)((xValue - startX) / adjustedPatchSizeX), patchQuantityX);
                            unsigned int patchZIndex = std::min((unsigned int)((zValue - startZ) / adjustedPatchSizeZ), patchQuantityZ);
                            unsigned int patchIndex = (patchZIndex * patchQuantityX) + patchXIndex;

                            leafGrassPatches[patchIndex]->addVertex(globalGrassVertex, grassNormal);
                        }
                    }
                });
            }
            std::ranges::for_each(threads, [](std::jthread& x){x.join();});

            createRenderers(leafGrassPatches);
            buildGrassQuadtree(std::move(leafGrassPatches), patchQuantityX, patchQuantityZ);
        }
    }

    unsigned int TerrainGrass::retrieveVertexIndex(const Point2<float>& localXzCoordinate) const {
        Point3 localCoordinate(localXzCoordinate.X, 0.0f, localXzCoordinate.Y);
        Point3<float> farLeftCoordinate = localCoordinate - mesh->getVertices()[0];

        float xInterval = mesh->getVertices()[1].X - mesh->getVertices()[0].X;
        unsigned int xIndex = std::clamp(MathFunction::roundToUInt(farLeftCoordinate.X / xInterval), 0u, mesh->getXSize() - 1);

        float zInterval = mesh->getVertices()[mesh->getXSize()].Z - mesh->getVertices()[0].Z;
        unsigned int zIndex = std::clamp(MathFunction::roundToUInt(farLeftCoordinate.Z / zInterval), 0u, mesh->getZSize() - 1);

        return xIndex + zIndex * mesh->getXSize();
    }

    void TerrainGrass::buildGrassQuadtree(std::vector<std::unique_ptr<TerrainGrassQuadtree>> leafGrassPatches, unsigned int leafQuantityX, unsigned int leafQuantityZ) {
        std::vector<std::unique_ptr<TerrainGrassQuadtree>> childrenGrassQuadtree = std::move(leafGrassPatches);
        unsigned int childrenNbQuadtreeX = leafQuantityX;
        unsigned int childrenNbQuadtreeZ = leafQuantityZ;

        unsigned int depth = grassQuadtreeDepth;
        while (depth >= 1) {
            auto depthNbQuadtreeX = (unsigned int)(MathFunction::pow(2, depth));
            auto depthNbQuadtreeZ = depthNbQuadtreeX;
            unsigned int depthNbQuadtree = depthNbQuadtreeX * depthNbQuadtreeZ;
            if (std::sqrt(childrenGrassQuadtree.size()) >= std::sqrt(depthNbQuadtree) * 2) {
                std::vector<std::unique_ptr<TerrainGrassQuadtree>> depthGrassQuadtree;
                depthGrassQuadtree.reserve(depthNbQuadtree);
                for (unsigned int i = 0; i < depthNbQuadtree; ++i) {
                    depthGrassQuadtree.push_back(std::make_unique<TerrainGrassQuadtree>());
                }

                for (unsigned int childZ = 0; childZ < childrenNbQuadtreeZ; ++childZ) {
                    for (unsigned int childX = 0; childX < childrenNbQuadtreeX; ++childX) {
                        auto xQuadtreeIndex = (unsigned int)(((float)depthNbQuadtreeX / (float)childrenNbQuadtreeX) * ((float)childX + 0.5f));
                        auto zQuadtreeIndex = (unsigned int)(((float)depthNbQuadtreeZ / (float)childrenNbQuadtreeZ) * ((float)childZ + 0.5f));

                        unsigned int quadtreeIndex = (zQuadtreeIndex * depthNbQuadtreeX) + xQuadtreeIndex;
                        unsigned int childQuadtreeIndex = (childZ * childrenNbQuadtreeZ) + childX;

                        assert(childrenGrassQuadtree[childQuadtreeIndex]);
                        depthGrassQuadtree[quadtreeIndex]->addChild(std::move(childrenGrassQuadtree[childQuadtreeIndex]));
                    }
                }

                childrenGrassQuadtree = std::move(depthGrassQuadtree);
                childrenNbQuadtreeX = depthNbQuadtreeX;
                childrenNbQuadtreeZ = depthNbQuadtreeZ;
            }

            depth--;
        }

        mainGrassQuadtree = std::make_unique<TerrainGrassQuadtree>(std::move(childrenGrassQuadtree));
    }

    void TerrainGrass::createRenderers(const std::vector<std::unique_ptr<TerrainGrassQuadtree>>& leafGrassPatches) {
        if (grassTexture && renderTarget) {
            for (auto& grassQuadtree : leafGrassPatches) {
                auto renderer = GenericRendererBuilder::create("grass", *renderTarget, *terrainGrassShader, ShapeType::POINT)
                        ->enableDepthTest()
                        ->enableDepthWrite()
                        ->disableCullFace()
                        ->addData(grassQuadtree->getGrassVertices())
                        ->addData(grassQuadtree->getGrassNormals())
                        ->addUniformData(sizeof(positioningData), &positioningData) //binding 0
                        ->addUniformData(sizeof(grassProperties), &grassProperties) //binding 1
                        ->addUniformData(sizeof(terrainPositioningData), &terrainPositioningData) //binding 2
                        ->addUniformData(sizeof(ambient), &ambient) //binding 3
                        ->addUniformTextureReader(TextureReader::build(grassTexture, TextureParam::build(TextureParam::EDGE_CLAMP, TextureParam::LINEAR, TextureParam::ANISOTROPY))) //binding 4
                        ->addUniformTextureReader(TextureReader::build(grassMaskTexture, TextureParam::buildLinear())) //binding 5
                        ->build();

                grassQuadtree->setRenderer(std::move(renderer));
            }
        }
    }

    std::vector<GenericRenderer*> TerrainGrass::getAllRenderers() const {
        std::vector<GenericRenderer*> renderers;

        if (mainGrassQuadtree != nullptr) {
            grassQuadtrees.clear();
            grassQuadtrees.push_back(mainGrassQuadtree.get());
            for (std::size_t i = 0; i < grassQuadtrees.size(); ++i) {
                const TerrainGrassQuadtree* grassQuadtree = grassQuadtrees[i];
                if (grassQuadtree->isLeaf() && grassQuadtree->getRenderer()) {
                    renderers.emplace_back(grassQuadtree->getRenderer());
                } else {
                    for (const auto& child : grassQuadtree->getChildren()) {
                        grassQuadtrees.push_back(child.get());
                    }
                }
            }
        }

        return renderers;
    }

    const std::string& TerrainGrass::getGrassTexture() const {
        return grassTextureFilename;
    }

    void TerrainGrass::setGrassTexture(std::string grassTextureFilename) {
        if (grassTextureFilename.empty() || FileUtil::isAbsolutePath(grassTextureFilename)) {
            this->grassTextureFilename = std::move(grassTextureFilename);
        } else {
            this->grassTextureFilename = FileSystem::instance().getResourcesDirectory() + std::move(grassTextureFilename);
        }

        if (this->grassTextureFilename.empty()) {
            grassTexture = nullptr;
        } else {
            grassTexture = ResourceRetriever::instance().getResource<Texture>(this->grassTextureFilename, {{"mipMap", "1"}});
        }
    }

    const std::string& TerrainGrass::getMaskTexture() const {
        return grassMaskFilename;
    }

    void TerrainGrass::setMaskTexture(std::string grassMaskFilename) {
        if (grassMaskFilename.empty() || FileUtil::isAbsolutePath(grassMaskFilename)) {
            this->grassMaskFilename = std::move(grassMaskFilename);
        } else {
            this->grassMaskFilename = FileSystem::instance().getResourcesDirectory() + std::move(grassMaskFilename);
        }

        if (this->grassMaskFilename.empty()) {
            std::vector<unsigned char> grassMaskColor({0});
            grassMaskTexture = Texture::build("default grass mask", 1, 1, TextureFormat::GRAYSCALE_8_INT, grassMaskColor.data(), TextureDataType::INT_8);
        } else {
            grassMaskTexture = ResourceRetriever::instance().getResource<Texture>(this->grassMaskFilename, {{"mipMap", "0"}});
        }
    }

    float TerrainGrass::getGrassDisplayDistance() const {
        return grassProperties.displayDistance;
    }

    void TerrainGrass::setGrassDisplayDistance(float grassDisplayDistance) {
        assert(grassDisplayDistance != 0.0f);
        grassProperties.displayDistance = grassDisplayDistance;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    float TerrainGrass::getGrassHeight() const {
        return grassProperties.height;
    }

    void TerrainGrass::setGrassHeight(float grassHeight) {
        grassProperties.height = grassHeight;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    float TerrainGrass::getGrassLength() const {
        return grassProperties.length;
    }

    void TerrainGrass::setGrassLength(float grassLength) {
        grassProperties.length = grassLength;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    unsigned int TerrainGrass::getNumGrassInTexture() const {
        return (unsigned int)grassProperties.numGrassInTex;
    }

    void TerrainGrass::setNumGrassInTexture(unsigned int numGrassInTex) {
        grassProperties.numGrassInTex = (int)numGrassInTex;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    float TerrainGrass::getGrassQuantity() const {
        return grassQuantity;
    }

    void TerrainGrass::setGrassQuantity(float grassQuantity) {
        this->grassQuantity = grassQuantity;

        generateGrass(mesh, terrainPosition);
    }

    Vector3<float> TerrainGrass::getWindDirection() const {
        return grassProperties.windDirection;
    }

    void TerrainGrass::setWindDirection(const Vector3<float>& windDirection) {
        grassProperties.windDirection = windDirection.normalize();

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    float TerrainGrass::getWindStrength() const {
        return grassProperties.windStrength;
    }

    void TerrainGrass::setWindStrength(float windStrength) {
        grassProperties.windStrength = windStrength;

        for (auto* renderer: getAllRenderers()) {
            renderer->updateUniformData(1, &grassProperties);
        }
    }

    void TerrainGrass::prepareRendering(unsigned int renderingOrder, const Camera& camera, float dt) {
        assert(bIsInitialized);

        if (grassTexture) {
            ScopeProfiler sp(Profiler::graphic(), "grassPreRender");

            positioningData.projectionViewMatrix = camera.getProjectionViewMatrix();
            positioningData.cameraPosition = camera.getPosition();
            positioningData.sumTimeStep += dt;

            grassQuadtrees.clear();
            grassQuadtrees.push_back(mainGrassQuadtree.get());

            for (std::size_t i = 0; i < grassQuadtrees.size(); ++i) {
                const TerrainGrassQuadtree* grassQuadtree = grassQuadtrees[i];
                const auto* grassQuadtreeBox = grassQuadtree->getBox();

                if (grassQuadtreeBox && camera.getFrustum().cutFrustum(grassProperties.displayDistance).collideWithAABBox(*grassQuadtreeBox)) {
                    if (grassQuadtree->isLeaf()) {
                        grassQuadtree->getRenderer()->updateUniformData(0, &positioningData);
                        grassQuadtree->getRenderer()->enableRenderer(renderingOrder);
                    } else {
                        for (const auto& child : grassQuadtree->getChildren()) {
                            grassQuadtrees.push_back(child.get());
                        }
                    }
                }
            }
        }
    }

}
