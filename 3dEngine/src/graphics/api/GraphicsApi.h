#pragma once

//choose rendering graphics API
#define VULKAN_GRAPHICS_API

#ifdef VULKAN_GRAPHICS_API
    #include <graphics/api/vulkan/setup/spi/SurfaceCreator.h>
    #include <graphics/api/vulkan/setup/VulkanService.h> //TODO remove ? (public method with Vk***)
    #include <graphics/api/vulkan/render/GenericRenderer.h>
    #include <graphics/api/vulkan/render/target/RenderTarget.h>
    #include <graphics/api/vulkan/render/target/OffscreenRender.h>
    #include <graphics/api/vulkan/render/target/ScreenRender.h>
    #include <graphics/api/vulkan/render/target/NullRenderTarget.h>
    #include <graphics/api/vulkan/render/shader/Shader.h>
    #include <graphics/api/vulkan/texture/Texture.h>
    #include <graphics/api/vulkan/texture/TextureParam.h>
#endif