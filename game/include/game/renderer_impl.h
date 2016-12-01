#include "game/renderer.h"

#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

C_HEADER_BEGIN

struct vulkan_context_t
{
    VkInstance instance;
};

struct vulkan_t
{
    void* module;

    PFN_vkCreateInstance                    vkCreateInstance;
    PFN_vkEnumerateInstanceLayerProperties  vkEnumerateInstanceLayerProperties;

    VkApplicationInfo application_info;
    struct vulkan_context_t context;
};

struct renderer_t
{
    uint32_t width, height;

    struct vulkan_t vk;
};
