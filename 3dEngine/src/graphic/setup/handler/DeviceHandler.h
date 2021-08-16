#pragma once

#include <vulkan/vulkan.h>

namespace urchin {

    struct PhysicalDeviceFeature {
        PhysicalDeviceFeature(unsigned long, std::string);

        unsigned long offset;
        std::string featureDescription;
    };

    struct PhysicalDeviceSuitability {
        PhysicalDeviceSuitability(VkPhysicalDevice, int);
        PhysicalDeviceSuitability(VkPhysicalDevice, std::string);

        VkPhysicalDevice physicalDevice;
        int score;
        std::optional<std::string> missingRequisiteDescription;
    };

    class DeviceHandler {
        public:
            DeviceHandler();
            DeviceHandler(const DeviceHandler&) = delete;

            void initializeDevices(VkInstance instance, VkSurfaceKHR);
            void cleanup();

            VkPhysicalDevice getPhysicalDevice() const;
            VkDevice getLogicalDevice() const;

        private:
            VkPhysicalDevice findPhysicalDevice(VkInstance instance);
            PhysicalDeviceSuitability retrievePhysicalDeviceSuitability(VkPhysicalDevice);
            bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice, const char*) const;

            VkDevice createLogicalDevice();

            std::vector<PhysicalDeviceFeature> physicalDeviceRequiredFeatures;
            std::vector<std::pair<const char*, std::string>> physicalDeviceRequiredExtensions;
            bool devicesInitialized;
            VkSurfaceKHR surface;

            VkPhysicalDevice physicalDevice;

            VkDevice logicalDevice;
    };

}
