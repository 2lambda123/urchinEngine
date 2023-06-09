#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <UrchinCommon.h>

namespace urchin {

    class DebugLabelHelper {
        public:
            enum ObjectType {
                PIPELINE,
                RENDER_PASS,
                SHADER,
                IMAGE,
                COMMAND_BUFFER,
                BUFFER
            };

            static void nameObject(ObjectType, void*, std::string_view);

            static void beginDebugRegion(VkCommandBuffer, const std::string&, const Vector4<float>&);
            static void endDebugRegion(VkCommandBuffer);

        private:
            static std::pair<VkObjectType, std::string> toVkObjectType(ObjectType);
    };

}
