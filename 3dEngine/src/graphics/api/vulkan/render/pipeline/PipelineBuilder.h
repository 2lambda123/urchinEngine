#pragma once

#include <memory>
#include <vulkan/vulkan.h>

#include <graphics/render/shader/ShaderDataContainer.h>
#include <graphics/render/data/DataContainer.h>
#include <graphics/render/data/ShapeType.h>
#include <graphics/render/data/PolygonMode.h>
#include <graphics/render/data/BlendFunction.h>
#include <graphics/api/vulkan/render/pipeline/Pipeline.h>
#include <graphics/api/vulkan/render/target/RenderTarget.h>
#include <graphics/api/vulkan/render/shader/Shader.h>

namespace urchin {

    class TextureReader;

    class PipelineBuilder {
        public:
            explicit PipelineBuilder(std::string);

            void setupRenderTarget(const RenderTarget&);
            void setupShader(const Shader&);
            void setupShapeType(const ShapeType&);
            void setupBlendFunctions(const std::vector<BlendFunction>&);
            void setupDepthOperations(bool, bool);
            void setupCallFaceOperation(bool);
            void setupPolygonMode(PolygonMode);
            void setupData(const std::vector<DataContainer>&, const DataContainer*);
            void setupUniform(const std::vector<ShaderDataContainer>&, const std::vector<std::vector<std::shared_ptr<TextureReader>>>&);

            std::shared_ptr<Pipeline> buildPipeline();

        private:
            void checkSetup() const;
            std::size_t computePipelineHash() const;

            void createDescriptorSetLayout(const std::shared_ptr<Pipeline>&) const;
            void createGraphicsPipeline(const std::shared_ptr<Pipeline>&);
            VkPrimitiveTopology shapeTypeToVulkanTopology() const;
            bool isShapeTypeListTopology() const;

            VkFormat getVulkanFormat(const DataContainer&, unsigned int&) const;
            VkBlendFactor toVkBlenderFactor(BlendFactor) const;

            std::string name;
            const RenderTarget* renderTarget;
            const Shader* shader;
            ShapeType shapeType;

            const std::vector<DataContainer>* data;
            const DataContainer* instanceData;
            const std::vector<ShaderDataContainer>* uniformData;
            const std::vector<std::vector<std::shared_ptr<TextureReader>>>* uniformTextureReaders;

            std::vector<BlendFunction> blendFunctions;
            bool depthTestEnabled;
            bool depthWriteEnabled;
            bool cullFaceEnabled;
            PolygonMode polygonMode;
    };

}
