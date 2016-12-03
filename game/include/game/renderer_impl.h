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
    VkApplicationInfo application_info;

    struct vulkan_context_t context;

    PFN_vkCreateInstance                    vkCreateInstance;
    PFN_vkDestroyInstance                   vkDestroyInstance;
    PFN_vkEnumerateInstanceLayerProperties  vkEnumerateInstanceLayerProperties;
};

struct renderer_t
{
    struct game_t* game;
    uint32_t width, height;
    struct vulkan_t vk;
};
