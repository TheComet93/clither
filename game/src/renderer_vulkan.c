#include "game/renderer_vulkan.h"
#include "game/renderer_impl.h"
#include "game/vulkan_utils.h"
#include "game/log.h"
#include "util/module_loader.h"
#include "util/memory.h"
#include <string.h>
#include <assert.h>

#if defined(UTIL_PLATFORM_LINUX)
#   define VULKAN_LIB "./libvulkan.so.1"
#elif defined(UTIL_PLATFORM_WINDOWS)
#   define VULKAN_LIB ".\\vulkan-1.dll"
#else
#   error Unsupported platform
#endif

/* ------------------------------------------------------------------------- */
/* Vulkan should use our debug memory allocation if enabled */
#if defined(VULKAN_USE_DEBUG_MALLOC)
void*
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

void
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
char
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
    LOAD_SYMBOL(vkEnumerateInstanceExtensionProperties);

    return 1;

    load_symbol_failed: vulkan_unload_library(renderer);
    open_module_failed: return 0;
}

/* ------------------------------------------------------------------------- */
void
vulkan_unload_library(struct renderer_t* renderer)
{
    assert(renderer);
    if(renderer->vk.module)
        module_close(renderer->vk.module);
    renderer->vk.module = NULL;
}

/* ------------------------------------------------------------------------- */
char
vulkan_create_instance(struct renderer_t* renderer)
{
    VkResult result;
    VkInstanceCreateInfo instance_info = {0};

    static const char* validation_layer_names[] = {
        "VK_LAYER_LUNARG_standard_validation",
        NULL
    };

    static const char* extension_names[] = {
        "VK_KHR_surface",
        "VK_KHR_xlib_surface",
        "VK_EXT_debug_report",
        NULL
    };

    renderer->vk.application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; /* sType is a member of all structs  */
    renderer->vk.application_info.pNext = NULL;                               /* as is pNext and flag              */
    renderer->vk.application_info.pApplicationName = "Clither";               /* The name of our application       */
    renderer->vk.application_info.pEngineName = NULL;                         /* The name of the engine            */
    renderer->vk.application_info.engineVersion = 1;                          /* The version of the engine         */
    renderer->vk.application_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);      /* The version of Vulkan we're using */

    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &renderer->vk.application_info;

    if(!vulkan_fill_in_validation_layer_info(renderer, validation_layer_names, &instance_info))
        return 0;
    if(!vulkan_fill_in_extension_info(renderer, extension_names, &instance_info))
        return 0;

    result = renderer->vk.vkCreateInstance(
            &instance_info,
            g_allocators_ptr,
            &renderer->vk.context.instance);

    vulkan_clean_up_extension_info(&instance_info);
    vulkan_clean_up_validation_layer_info(&instance_info);

    if(result != VK_SUCCESS)
    {
        log_message(LOG_FATAL, renderer->game, "[renderer] Failed to create vulkan instance: %s", vulkan_result_to_string(result));
        return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------------- */
void
vulkan_destroy_instance(struct renderer_t* renderer)
{
    renderer->vk.vkDestroyInstance(renderer->vk.context.instance, g_allocators_ptr);
}

/* ------------------------------------------------------------------------- */
char
vulkan_fill_in_validation_layer_info(const struct renderer_t* renderer,
                                     const char** validation_layer_names,
                                     VkInstanceCreateInfo* instance_info)
{
    const char** layer_name_it;
    const char** ppEnabledLayerNames;
    char layer_found;
    uint32_t layer_it;
    uint32_t layer_count;
    VkLayerProperties* layers_available;

    assert(instance_info->enabledLayerCount == 0);
    assert(instance_info->ppEnabledLayerNames == NULL);

    /*
     * Enumerate available layers, load their names into a buffer.
     */
    renderer->vk.vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if(layer_count == 0)
    {
        log_message(LOG_WARNING, renderer->game, "[renderer] 0 validation layers were found; none will be loaded.");
        return 1;
    }

    layers_available = (VkLayerProperties*)MALLOC(layer_count * sizeof(*layers_available), "vulkan_fill_in_validation_layers()");
    if(layers_available == NULL)
        return 0;
    renderer->vk.vkEnumerateInstanceLayerProperties(&layer_count, layers_available);

    /*
     * Need a separate buffer in which we insert layer names
     * that will be loaded. This buffer is passed to the instance_info struct.
     */
    ppEnabledLayerNames = (const char**)MALLOC(layer_count * sizeof(char*), "vulkan_fill_in_validation_layers()");
    if(ppEnabledLayerNames == NULL)
    {
        FREE(layers_available);
        return 0;
    }

    /* Log which layers were found */
    log_message(LOG_INFO, renderer->game, "[renderer] Found the following vulkan layers");
    for(layer_it = 0; layer_it != layer_count; ++layer_it)
        log_message(LOG_INFO, renderer->game, "[renderer]   + %s", layers_available[layer_it].layerName);

    /*
     * Match found layers with the requested layers. Insert
     * matches into ppEnabledLayerNames.
     */
    for(layer_name_it = validation_layer_names; *layer_name_it != NULL; ++layer_name_it)
    {
        layer_found = 0;
        for(layer_it = 0; layer_it != layer_count; ++layer_it)
        {
            if(strcmp(*layer_name_it, layers_available[layer_it].layerName) == 0)
            {
                log_message(LOG_INFO, renderer->game, "[renderer] Loading layer %s", *layer_name_it);
                ppEnabledLayerNames[instance_info->enabledLayerCount++] = *layer_name_it;

                layer_found = 1;
                break;
            }
        }

        if(layer_found == 0)
            log_message(LOG_WARNING, renderer->game, "[renderer] Couldn't find layer with name %s", *layer_name_it);
    }

    instance_info->ppEnabledLayerNames = ppEnabledLayerNames;

    FREE(layers_available);

    return 1;
}

/* ------------------------------------------------------------------------- */
void
vulkan_clean_up_validation_layer_info(VkInstanceCreateInfo* instance_info)
{
    if(instance_info->enabledLayerCount == 0)
    {
        assert(instance_info->ppEnabledLayerNames == NULL);
        return;
    }

    FREE((void*)instance_info->ppEnabledLayerNames);
    instance_info->enabledLayerCount = 0;
    instance_info->ppEnabledLayerNames = NULL;
}

/* ------------------------------------------------------------------------- */
char
vulkan_fill_in_extension_info(const struct renderer_t* renderer,
                              const char** extension_names,
                              VkInstanceCreateInfo* instance_info)
{
    const char** extension_name_it;
    const char** ppEnabledExtensionNames;
    uint32_t extension_it;
    uint32_t extension_count;
    VkExtensionProperties* extensions_available;
    char extension_found;

    assert(instance_info->enabledExtensionCount == 0);
    assert(instance_info->ppEnabledExtensionNames == NULL);

    /*
     * Enumerate available extensions, load their names into a buffer.
     */
    renderer->vk.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    if(extension_count == 0)
    {
        log_message(LOG_WARNING, renderer->game, "[renderer] No extensions were found; none will be loaded.");
        return 1;
    }

    extensions_available = (VkExtensionProperties*)MALLOC(extension_count * sizeof(*extensions_available), "vulkan_fill_in_extension_info()");
    if(extensions_available == NULL)
        return 0;
    renderer->vk.vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions_available);

    /*
     * Need a separate buffer in which we insert extension names
     * that will be loaded. This buffer is passed to the instance_info struct.
     */
    ppEnabledExtensionNames = (const char**)MALLOC(extension_count * sizeof(char*), "vulkan_fill_in_extension_info()");
    if(ppEnabledExtensionNames == NULL)
    {
        FREE(extensions_available);
        return 0;
    }

    /* Log which extensions were found */
    log_message(LOG_INFO, renderer->game, "[renderer] Found the following vulkan extensions");
    for(extension_it = 0; extension_it != extension_count; ++extension_it)
        log_message(LOG_INFO, renderer->game, "[renderer]   + %s", extensions_available[extension_it].extensionName);

    /*
     * Match found extensions with the requested extensions. Insert
     * matches into ppEnabledExtensionNames.
     */
    for(extension_name_it = extension_names; *extension_name_it != NULL; ++extension_name_it)
    {
        extension_found = 0;
        for(extension_it = 0; extension_it != extension_count; ++extension_it)
        {
            if(strcmp(*extension_name_it, extensions_available[extension_it].extensionName) == 0)
            {
                log_message(LOG_INFO, renderer->game, "[renderer] Loading extension %s", *extension_name_it);
                ppEnabledExtensionNames[instance_info->enabledExtensionCount++] = *extension_name_it;

                extension_found = 1;
                break;
            }
        }

        if(extension_found == 0)
            log_message(LOG_WARNING, renderer->game, "[renderer] Couldn't find extension with name %s", *extension_name_it);
    }

    instance_info->ppEnabledExtensionNames = ppEnabledExtensionNames;

    FREE(extensions_available);

    return 1;
}

/* ------------------------------------------------------------------------- */
void
vulkan_clean_up_extension_info(VkInstanceCreateInfo* instance_info)
{
    if(instance_info->enabledExtensionCount == 0)
    {
        assert(instance_info->ppEnabledExtensionNames == NULL);
        return;
    }

    FREE((void*)instance_info->ppEnabledExtensionNames);
    instance_info->enabledExtensionCount = 0;
    instance_info->ppEnabledExtensionNames = NULL;
}
