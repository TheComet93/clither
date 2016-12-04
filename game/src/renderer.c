#include "game/renderer_impl.h"
#include "game/vulkan_utils.h"
#include "game/log.h"
#include "util/module_loader.h"
#include "util/memory.h"
#include <stdio.h>
#include <stdlib.h>
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
load_vulkan_library(struct vulkan_t* vk);

static char
load_validation_layer(struct vulkan_t* vk);

static void
unload_vulkan_library(struct vulkan_t* vk);

/* ------------------------------------------------------------------------- */
/* Vulkan should use our debug memory allocation */

/*
 * PFN_vkAllocationFunction implementation
 */
static void*
allocation_function(void* pUserData, size_t  size,  size_t  alignment, VkSystemAllocationScope allocationScope)
{
    /* Ignore alignment, for while */
    return MALLOC(size, "allocationFunction() (vulkan malloc wrapper)"); /*_aligned_malloc(size, alignment); */
}

void* reallocation_function(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
    printf("pAllocator's REallocationFunction: size %lu, alignment %lu, allocationScope %d \n",
    size, alignment, allocationScope);
    return realloc(pOriginal, size);
 }

/*
 * The PFN_vkFreeFunction implementation
 */
static void
free_function(void* pUserData, void* pMemory)
{
    FREE(pMemory);
}

VkAllocationCallbacks g_vkAllocators = {
    NULL,                 /* pUserData;             */
    allocation_function,  /* pfnAllocation;         */
    reallocation_function,/* pfnReallocation;       */
    free_function,        /* pfnFree;               */
    NULL,                 /* pfnInternalAllocation; */
    NULL                  /* pfnInternalFree;       */
};

/* ------------------------------------------------------------------------- */
struct renderer_t*
renderer_create(struct game_t* game)
{
    struct renderer_t* renderer;
    if(!(renderer = (struct renderer_t*)MALLOC(sizeof *renderer, "renderer_create()")))
        return NULL;

    if(!renderer_init(renderer, game))
    {
        FREE(renderer);
        return NULL;
    }

    return renderer;
}

/* ------------------------------------------------------------------------- */
char
renderer_init(struct renderer_t* renderer, struct game_t* game)
{
    assert(renderer);

    renderer->game = game;
    renderer->width = 800;
    renderer->height = 600;

    if(!load_vulkan_library(&renderer->vk))
        goto load_vulkan_library_failed;

    renderer->vk.application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; /* sType is a member of all structs  */
    renderer->vk.application_info.pNext = NULL;                               /* as is pNext and flag              */
    renderer->vk.application_info.pApplicationName = "Clither";               /* The name of our application       */
    renderer->vk.application_info.pEngineName = NULL;                         /* The name of the engine            */
    renderer->vk.application_info.engineVersion = 1;                          /* The version of the engine         */
    renderer->vk.application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);      /* The version of Vulkan we're using */

    {
        VkResult result;
        VkInstanceCreateInfo instanceInfo = {0};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &renderer->vk.application_info;
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = NULL;
        instanceInfo.enabledExtensionCount = 0;
        instanceInfo.ppEnabledExtensionNames = NULL;

        result = renderer->vk.vkCreateInstance(
                &instanceInfo,
                &g_vkAllocators,
                &renderer->vk.context.instance);
        if(result != VK_SUCCESS)
        {
            log_message(LOG_FATAL, game, "Failed to create vulkan instance:\n%s", vulkan_result_to_string(result));
            goto create_vulkan_instance_failed;
        }
    }

    load_validation_layer(&renderer->vk);

    return 1;

    create_vulkan_instance_failed : unload_vulkan_library(&renderer->vk);
    load_vulkan_library_failed    : return 0;
}

/* ------------------------------------------------------------------------- */
void
renderer_destroy(struct renderer_t* renderer)
{
    assert(renderer);

    renderer->vk.vkDestroyInstance(renderer->vk.context.instance, &g_vkAllocators);
    unload_vulkan_library(&renderer->vk);

    FREE(renderer);
}

/* ------------------------------------------------------------------------- */
/* STATIC FUNCTIONS */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static char
load_vulkan_library(struct vulkan_t* vk)
{
    vk->module = module_open(VULKAN_LIB);
    if(vk->module == NULL)
        goto open_module_failed;

    if(!(*(void**)&vk->vkCreateInstance                   = module_sym(vk->module, "vkCreateInstance")))                   goto load_symbol_failed;
    if(!(*(void**)&vk->vkDestroyInstance                  = module_sym(vk->module, "vkDestroyInstance")))                  goto load_symbol_failed;
    if(!(*(void**)&vk->vkEnumerateInstanceLayerProperties = module_sym(vk->module, "vkEnumerateInstanceLayerProperties"))) goto load_symbol_failed;

    return 1;

    load_symbol_failed: unload_vulkan_library(vk);
    open_module_failed: return 0;
}

/* ------------------------------------------------------------------------- */
static char
load_validation_layer(struct vulkan_t* vk)
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
unload_vulkan_library(struct vulkan_t* vk)
{
    assert(vk);
    if(vk->module)
        module_close(vk->module);
    vk->module = NULL;
}
