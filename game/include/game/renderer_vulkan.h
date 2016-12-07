#include "game/config.h"

C_HEADER_BEGIN

#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

struct renderer_t;

char
vulkan_load_library(struct renderer_t* renderer);

void
vulkan_unload_library(struct renderer_t* renderer);

char
vulkan_create_instance(struct renderer_t* renderer);

void
vulkan_destroy_instance(struct renderer_t* renderer);

char
vulkan_fill_in_validation_layer_info(const struct renderer_t* renderer,
                                 const char** validation_layer_names,
                                 VkInstanceCreateInfo* instance_info);

void
vulkan_clean_up_validation_layer_info(VkInstanceCreateInfo* instance_info);

char
vulkan_fill_in_extension_info(const struct renderer_t* renderer,
                              const char** extension_names,
                              VkInstanceCreateInfo* instance_info);

void
vulkan_clean_up_extension_info(VkInstanceCreateInfo* instance_info);

C_HEADER_END
