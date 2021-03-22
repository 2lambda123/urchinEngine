#ifndef URCHINENGINE_IMAGEHELPER_H
#define URCHINENGINE_IMAGEHELPER_H

#include <vulkan/vulkan.h>

VK_DEFINE_HANDLE(VmaAllocation)

namespace urchin {

    class ImageHelper {
        public:
            static VkImage createImage(uint32_t, uint32_t, uint32_t, uint32_t, bool, VkFormat, VkImageUsageFlags, VmaAllocation&);
            static VkImageView createImageView(VkImage, VkImageViewType, VkFormat, VkImageAspectFlags, uint32_t, uint32_t);

        private:
            static void checkFormatSupport(VkFormat, VkImageTiling, VkFormatFeatureFlags);
            static VkFormatFeatureFlags usageFlagToFeatureFlag(VkImageUsageFlags);
    };

}

#endif
