#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

/*!
 * @brief Converts a vulkan result into a human readable string.
 */
const char*
vulkan_result_to_string(VkResult result);
