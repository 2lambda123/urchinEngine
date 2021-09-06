#include <scene/renderer3d/model/builder/ModelBuilder.h>

namespace urchin {

    ModelBuilder::ModelBuilder(std::string materialFilename) :
            materialFilename(std::move(materialFilename)) {

    }

    std::unique_ptr<Model> ModelBuilder::newModel(const std::string& meshesName, const std::vector<Point3<float>>& vertices,
                                                  const std::vector<unsigned int>& trianglesIndices, const std::vector<Point2<float>>& texCoords) const {
        std::vector<std::unique_ptr<const ConstMesh>> constMeshesVector;
        constMeshesVector.push_back(buildConstMesh(vertices, trianglesIndices, texCoords));
        auto constMeshes = ConstMeshes::fromMemory(meshesName, std::move(constMeshesVector));

        auto meshes = std::make_unique<Meshes>(std::move(constMeshes));
        return Model::fromMemory(std::move(meshes));
    }

    std::unique_ptr<const ConstMesh> ModelBuilder::buildConstMesh(const std::vector<Point3<float>>& vertices, const std::vector<unsigned int>& trianglesIndices,
                                                                  const std::vector<Point2<float>>& texCoords) const {
        if (vertices.size() != texCoords.size()) {
            throw std::runtime_error("Vertices (" + std::to_string(vertices.size()) + ") must have exactly one UV coordinate (" + std::to_string(texCoords.size()) + ")");
        } else if (trianglesIndices.size() % 3 != 0) {
            throw std::runtime_error("Triangle indices must be a multiple of three");
        }

        std::vector<Vertex> modelVertices;
        modelVertices.reserve(vertices.size());
        std::vector<Weight> modelWeights;
        modelWeights.reserve(vertices.size());
        std::vector<Point2<float>> modelUvs;
        modelUvs.reserve(texCoords.size());

        unsigned int verticesGroupId = 0;
        int weightStart = 0;

        for (std::size_t i = 0;  i< vertices.size(); ++i) {
            Vertex modelVertex = {};
            modelVertex.linkedVerticesGroupId = verticesGroupId++;
            modelVertex.weightStart = weightStart++;
            modelVertex.weightCount = 1;
            modelVertices.push_back(modelVertex);

            modelUvs.push_back(texCoords[i]);

            Weight modelWeight = {};
            modelWeight.bone = 0;
            modelWeight.bias = 1;
            modelWeight.pos = vertices[i];
            modelWeights.push_back(modelWeight);
        }

        std::vector<Bone> modelBaseSkeleton;
        Bone modelBone = {};
        modelBone.name = "Bone";
        modelBone.parent = -1;
        modelBone.pos = Point3<float>(0.0f, 0.0f, 0.0f);
        modelBone.orient = Quaternion<float>();
        modelBaseSkeleton.push_back(modelBone);

        return std::make_unique<ConstMesh>(materialFilename, modelVertices, texCoords, trianglesIndices, modelWeights, modelBaseSkeleton);
    }
    
}