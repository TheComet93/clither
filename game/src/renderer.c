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
vulkan_load_library(struct renderer_t* renderer);

static void
vulkan_unload_library(struct renderer_t* renderer);

static char
vulkan_create_instance(struct renderer_t* renderer);

static void
vulkan_destroy_instance(struct renderer_t* renderer);

/* ------------------------------------------------------------------------- */
/* Vulkan should use our debug memory allocation if enabled */
#if defined(VULKAN_USE_DEBUG_MALLOC)
static void*
allocation_function(void* pUserData, size_t  size,  size_t  alignment, VkSystemAllocationScope allocationScope)
{
    /* Ignore alignment, for while */
    if(size == 0)
        return (void*)0;
    return MALLOC(size, "allocationFunction() (vulkan malloc wrapper)"); /*_aligned_malloc(size, alignment); */
}

void* reallocation_function(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
    printf("pAllocator's REallocationFunction: size %lu, alignment %lu, allocationScope %d \n",
    size, alignment, allocationScope);
    assert(0);
    return realloc(pOriginal, size);
 }

static void
free_function(void* pUserData, void* pMemory)
{
    if(pMemory)
        FREE(pMemory);
}

VkAllocationCallbacks g_allocators = {
    NULL,                 /* pUserData;             */
    allocation_function,  /* pfnAllocation;         */
    reallocation_function,/* pfnReallocation;       */
    free_function,        /* pfnFree;               */
    NULL,                 /* pfnInternalAllocation; */
    NULL                  /* pfnInternalFree;       */
};
VkAllocationCallbacks* g_allocators_ptr = &g_allocators;
#else /* VULKAN_USE_DEBUG_MALLOC */
VkAllocationCallbacks* g_allocators_ptr = NULL;
#endif /* VULKAN_USE_DEBUG_MALLOC */

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

    /* most functions rely on unused pointers to be NULL */
    memset(renderer, 0, sizeof *renderer);

    renderer->game = game;
    renderer->width = 800;
    renderer->height = 600;

    if(!vulkan_load_library(renderer))
        goto load_vulkan_library_failed;

    if(!vulkan_create_instance(renderer))
        goto create_vulkan_instance_failed;

    return 1;

    create_vulkan_instance_failed : vulkan_unload_library(renderer);
    load_vulkan_library_failed    : return 0;
}

/* ------------------------------------------------------------------------- */
void
renderer_destroy(struct renderer_t* renderer)
{
    assert(renderer);

    vulkan_destroy_instance(renderer);
    vulkan_unload_library(renderer);

    FREE(renderer);
}

/* ------------------------------------------------------------------------- */
/* STATIC FUNCTIONS */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static char
vulkan_load_library(struct renderer_t* renderer)
{
    renderer->vk.module = module_open(VULKAN_LIB);
    if(renderer->vk.module == NULL)
        goto open_module_failed;

    /* Make our lives a little bit easier */
#define LOAD_SYMBOL(symbol)                        \
    if(!(*(void**)&renderer->vk.symbol =           \
        module_sym(renderer->vk.module, #symbol))) \
        goto load_symbol_failed

    LOAD_SYMBOL(vkCreateInstance);
    LOAD_SYMBOL(vkDestroyInstance);
    LOAD_SYMBOL(vkEnumerateInstanceLayerProperties);

    return 1;

    load_symbol_failed: vulkan_unload_library(renderer);
    open_module_failed: return 0;
}

/* ------------------------------------------------------------------------- */
static char
vulkan_create_instance(struct renderer_t* renderer)
{
    VkResult result;
    VkInstanceCreateInfo instance_info = {0};

    renderer->vk.application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; /* sType is a member of all structs  */
    renderer->vk.application_info.pNext = NULL;                               /* as is pNext and flag              */
    renderer->vk.application_info.pApplicationName = "Clither";               /* The name of our application       */
    renderer->vk.application_info.pEngineName = NULL;                         /* The name of the engine            */
    renderer->vk.application_info.engineVersion = 1;                          /* The version of the engine         */
    renderer->vk.application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);      /* The version of Vulkan we're using */

    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &renderer->vk.application_info;

    /*
     * Load validation layers before creating the instance.
     */
    {
        const char** layer_name;
        char layer_found;
        uint32_t layer_id;
        uint32_t layer_count;
        VkLayerProperties* layers_available;

        static const char* validation_layer_names[] = {
            "VK_LAYER_LUNARG_standard_validation",
            NULL
        };

        renderer->vk.vkEnumerateInstanceLayerProperties(&layer_count, NULL);
        if(layer_count == 0)
        {
            log_message(LOG_WARNING, renderer->game, "[renderer] 0 validation layers were found; none will be loaded.");
            return 1;
        }

        layers_available = (VkLayerProperties*)MALLOC(layer_count * sizeof(*layers_available), "vulkan_load_validation_layers()");
        if(layers_available == NULL)
            return 0;
        renderer->vk.vkEnumerateInstanceLayerProperties(&layer_count, layers_available);

        log_message(LOG_INFO, renderer->game, "[renderer] Found the following vulkan layers");
        log_message(LOG_INFO, renderer->game, "[renderer] (+) layer will be loaded  (-) layer won't be loaded");
        for(layer_id = 0; layer_id != layer_count; ++layer_id)
        {
            layer_found = 0;
            for(layer_name = validation_layer_names; *layer_name != NULL; ++layer_name)
            {
                if(strcmp(*layer_name, layers_available[layer_id].layerName) == 0)
                {
                    layer_found = 1;
                    break;
                }
            }
            log_message(LOG_INFO, renderer->game, "[renderer]   %s %s", (layer_found ? "+" : "-"), layers_available[layer_id].layerName);
        }
    }

    result = renderer->vk.vkCreateInstance(
            &instance_info,
            g_allocators_ptr,
            &renderer->vk.context.instance);

    if(result != VK_SUCCESS)
    {
        log_message(LOG_FATAL, renderer->game, "Failed to create vulkan instance:\n%s", vulkan_result_to_string(result));
        return 0;;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
static void
vulkan_destroy_instance(struct renderer_t* renderer)
{
    renderer->vk.vkDestroyInstance(renderer->vk.context.instance, g_allocators_ptr);
}

/* ------------------------------------------------------------------------- */
static void
vulkan_unload_library(struct renderer_t* renderer)
{
    assert(renderer);
    if(renderer->vk.module)
        module_close(renderer->vk.module);
    renderer->vk.module = NULL;
}
