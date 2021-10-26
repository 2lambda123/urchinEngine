#include <cstring>

#include <libs/vma/vk_mem_alloc.h>
#include <api/render/handler/BufferHandler.h>
#include <api/setup/GraphicService.h>
#include <api/helper/BufferHelper.h>
#include <api/helper/CommandBufferHelper.h>

namespace urchin {

    BufferHandler::BufferHandler() :
            isInitialized(false),
            bufferType(BufferType::VERTEX),
            dataSize(0),
            bufferKind(BufferKind::STATIC),
            stagingBuffer(nullptr),
            stagingBufferMemory(nullptr),
            buffer(nullptr),
            bufferMemory(nullptr) {

    }

    BufferHandler::~BufferHandler() {
        if (isInitialized) {
            Logger::instance().logWarning("Buffer handler not cleanup before destruction");
            cleanup();
        }
    }

    void BufferHandler::initialize(BufferType bufferType, BufferKind initialBufferKind, std::size_t dataSize, const void* dataPtr) {
        assert(!isInitialized);
        assert(dataSize > 0);

        this->bufferType = bufferType;
        this->bufferKind = initialBufferKind; //static buffer are automatically changed to dynamic buffer in case of update
        this->dataSize = dataSize;

        createOrRefreshBuffers(dataPtr);

        isInitialized = true;
    }

    void BufferHandler::cleanup() {
        assert(isInitialized);

        deleteBuffer();

        isInitialized = false;
    }

    VkBuffer BufferHandler::getBuffer() const {
        assert(isInitialized);
        return buffer;
    }

    std::size_t BufferHandler::getDataSize() const {
        assert(isInitialized);
        return dataSize;
    }

    BufferHandler::BufferKind BufferHandler::getBufferKind() const {
        assert(isInitialized);
        return bufferKind;
    }

    bool BufferHandler::updateData(std::size_t dataSize, const void *dataPtr) {
        assert(isInitialized);
        assert(dataPtr != nullptr);

        bool dataSizeAltered = this->dataSize != dataSize;
        bool newBufferCreated = false;

        this->dataSize = dataSize;

        if (bufferKind == BufferKind::STATIC) {
            bufferKind = BufferKind::DYNAMIC;
            createOrRefreshBuffers(dataPtr);
            newBufferCreated = true;
        } else {
            if (dataSizeAltered) {
                createOrRefreshBuffers(dataPtr);
                newBufferCreated = true;
            } else {
                updateBuffer(dataPtr);
            }
        }

        return newBufferCreated;
    }

    void BufferHandler::createOrRefreshBuffers(const void* dataPtr) {
        auto allocator = GraphicService::instance().getAllocator();
        auto bufferSize = static_cast<VkDeviceSize>(dataSize);

        deleteBuffer();

        VkBufferUsageFlagBits usageType = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        if (bufferType == BufferType::VERTEX) {
            usageType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        } else if (bufferType == BufferType::INDEX) {
            usageType = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        } else if (bufferType == BufferType::UNIFORM) {
            usageType = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (bufferKind == BufferKind::STATIC) {
            if (!dataPtr) {
                throw std::runtime_error("Data must be provided at initialization to build a static buffer");
            }
            stagingBuffer = BufferHelper::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

            void *dataDestination;
            vmaMapMemory(allocator, stagingBufferMemory, &dataDestination);
            {
                std::memcpy(dataDestination, dataPtr, dataSize);
            }
            vmaUnmapMemory(allocator, stagingBufferMemory);

            buffer = BufferHelper::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageType, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferMemory);
            copyBuffer(stagingBuffer, buffer, bufferSize);

            vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
        } else if (bufferKind == BufferKind::DYNAMIC) {
            buffer = BufferHelper::createBuffer(bufferSize, usageType, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferMemory);

            if (dataPtr) {
                updateBuffer(dataPtr);
            }
        } else {
            throw std::runtime_error("Unknown buffer kind: " + std::to_string(bufferKind));
        }
    }

    void BufferHandler::deleteBuffer() {
        auto allocator = GraphicService::instance().getAllocator();

        if (buffer != nullptr) {
            vmaDestroyBuffer(allocator, buffer, nullptr);
            buffer = nullptr;
        }
        if (bufferMemory != nullptr) {
            vmaFreeMemory(allocator, bufferMemory);
            bufferMemory = nullptr;
        }
    }

    void BufferHandler::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = CommandBufferHelper::beginSingleTimeCommands();
        {
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        }
        CommandBufferHelper::endSingleTimeCommands(commandBuffer);
    }

    void BufferHandler::updateBuffer(const void *dataPtr) {
        void *dataDestination;
        vmaMapMemory(GraphicService::instance().getAllocator(), bufferMemory, &dataDestination);
        {
            std::memcpy(dataDestination, dataPtr, dataSize);
        }
        vmaUnmapMemory(GraphicService::instance().getAllocator(), bufferMemory);
    }

}
