#ifndef URCHINENGINE_BUFFERHANDLER_H
#define URCHINENGINE_BUFFERHANDLER_H

#include <cstdlib>
#include <vulkan/vulkan.h>

VK_DEFINE_HANDLE(VmaAllocation)

namespace urchin {

    class BufferHandler {
        public:
            enum BufferType {
                VERTEX,
                INDEX,
                UNIFORM
            };
            enum BufferKind {
                STATIC,
                DYNAMIC
            };

            BufferHandler();
            ~BufferHandler();

            void initialize(BufferType, std::size_t, const void* dataPtr = nullptr);
            void cleanup();

            VkBuffer getBuffer() const;
            std::size_t getDataSize() const;
            BufferKind getBufferKind() const;

            bool updateData(const void *);

        private:
            void createOrUpdateBuffer(const void *);
            void deleteBuffer();
            static void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
            void updateBuffer(const void *);

            bool isInitialized;

            BufferType bufferType;
            std::size_t dataSize;
            BufferKind bufferKind;

            VkBuffer stagingBuffer; //GPU buffer accessible from CPU
            VmaAllocation stagingBufferMemory;
            VkBuffer buffer; //GPU buffer accessible only from stagingBuffer
            VmaAllocation bufferMemory;
    };

}

#endif
