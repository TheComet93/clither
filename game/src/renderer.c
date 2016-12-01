#include "game/renderer_impl.h"

#include "util/module_loader.h"
#include "util/memory.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#if defined(UTIL_PLATFORM_LINUX)
#   define VULKAN_LIB "./libvulkan.so.1"
#elif defined(UTIL_PLATFORM_WINDOWS)
#   define VULKAN_LIB ".\\vulkan-1.dll"
#else
#   error Unsupported platform
#endif

static char
checkVulkanResult(VkResult result, char* msg);

static char
loadVulkanLibrary(struct vulkan_t* vk);

static char
loadValidationLayer(struct vulkan_t* vk);

static void
unloadVulkanLibrary(struct vulkan_t* vk);

/* ------------------------------------------------------------------------- */
/* Vulkan should use our debug memory allocation */

/*
 * PFN_vkAllocationFunction implementation
 */
static void*
allocationFunction(void* pUserData, size_t  size,  size_t  alignment, VkSystemAllocationScope allocationScope)
{
    /* Ignore alignment, for while */
    return MALLOC(size); /*_aligned_malloc(size, alignment); */
}

/*
 * The PFN_vkFreeFunction implementation
 */
static void
freeFunction(void* pUserData, void* pMemory)
{
    FREE(pMemory);
}

VkAllocationCallbacks g_vkAllocators = {
    NULL,                 /* pUserData;             */
    allocationFunction,   /* pfnAllocation;         */
    NULL,                 /* pfnReallocation;       */
    freeFunction,         /* pfnFree;               */
    NULL,                 /* pfnInternalAllocation; */
    NULL                  /* pfnInternalFree;       */
};

/* ------------------------------------------------------------------------- */
struct renderer_t*
renderer_create(void)
{
    struct renderer_t* renderer;
    if(!(renderer = (struct renderer_t*)MALLOC(sizeof *renderer)))
        return NULL;
    renderer_init(renderer);
    return renderer;
}

/* ------------------------------------------------------------------------- */
char
renderer_init(struct renderer_t* renderer)
{
    assert(renderer);
    memset(renderer, 0, sizeof *renderer);

    if(!loadVulkanLibrary(&renderer->vk))
        return 0;

    renderer->vk.application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; /* sType is a member of all structs  */
    renderer->vk.application_info.pNext = NULL;                               /* as is pNext and flag              */
    renderer->vk.application_info.pApplicationName = "Clither";               /* The name of our application       */
    renderer->vk.application_info.pEngineName = NULL;                         /* The name of the engine            */
    renderer->vk.application_info.engineVersion = 1;                          /* The version of the engine         */
    renderer->vk.application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);      /* The version of Vulkan we're using */

    {
        VkInstanceCreateInfo instanceInfo = {0};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &renderer->vk.application_info;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = NULL;
        instanceInfo.enabledExtensionCount = 0;
        instanceInfo.ppEnabledExtensionNames = NULL;

        if(!checkVulkanResult(
            renderer->vk.vkCreateInstance(
                &instanceInfo,
                &g_vkAllocators,
                &renderer->vk.context.instance),
            "Failed to create vulkan instance"))
            return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
void
renderer_destroy(struct renderer_t* renderer)
{
    assert(renderer);

    unloadVulkanLibrary(&renderer->vk);

    FREE(renderer);
}

/* ------------------------------------------------------------------------- */
/* STATIC FUNCTIONS */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static char
checkVulkanResult(VkResult result, char* msg)
{
    if(result != VK_SUCCESS)
    {
        fprintf(stderr, "%s", msg);
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static char
loadVulkanLibrary(struct vulkan_t* vk)
{
    vk->module = module_open(VULKAN_LIB);
    if(vk->module == NULL)
        goto open_module_failed;

    if(!(vk->vkCreateInstance                   = *(PFN_vkCreateInstance*)                  module_sym(vk->module, "vkCreateInstance")))                   goto load_symbol_failed;
    if(!(vk->vkEnumerateInstanceLayerProperties = *(PFN_vkEnumerateInstanceLayerProperties*)module_sym(vk->module, "vkEnumerateInstanceLayerProperties"))) goto load_symbol_failed;

    load_symbol_failed: unloadVulkanLibrary(vk);
    open_module_failed: return 0;
}

/* ------------------------------------------------------------------------- */
static char
loadValidationLayer(struct vulkan_t* vk)
{
    uint32_t layer_count = 0;
    vk->vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if(layer_count == 0)
    {
        return 1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static void
unloadVulkanLibrary(struct vulkan_t* vk)
{
    assert(vk);
    if(vk->module)
        module_close(vk->module);
    vk->module = NULL;
}
