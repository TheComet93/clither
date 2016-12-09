#include "game/config.h"

C_HEADER_BEGIN

#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"

struct renderer_t;

/**
 * @brief Opens the vulkan shared library and extracts the relevant function
 * pointers and stores them into the renderer struct. The library must not have
 * been opened previously or this function will fail.
 * @param renderer A renderer object to store the vulkan stuff into.
 * @return Returns non-zero if successful, 0 if otherwise.
 */
char
vulkan_load_library(struct renderer_t* renderer);

/**
 * @brief Unloads the vulkan shared library.
 * @note This doesn't perform any cleanup other than closing the library. It is
 * expected the user calls the appropriate destroy functions.
 */
void
vulkan_unload_library(struct renderer_t* renderer);

/**
 * @brief Instantiates the vulkan instance. Currently, settings are hard-coded.
 * @note The vulkan library must have been loaded beforehand.
 * @return Returns non-zero if successful, 0 if otherwise.
 */
char
vulkan_create_instance(struct renderer_t* renderer);

/**
 * @brief Destroys a previously created vulkan instance.
 */
void
vulkan_destroy_instance(struct renderer_t* renderer);

/**
 * @brief Attempts to load the specified list of validation layers and fills
 * in the appropriate info into instance_info. If any of the specified layers
 * were not found, then a warning is written to the log and loading that
 * specific layer is skipped without error.
 * @note vulkan_clean_up_validation_layer_info() must be called to clean up
 * any buffers allocated when this function returns successfully.
 * @param renderer The renderer instance. Is not modified.
 * @param validation_layer_names A list of char-arrays containing the names of
 * the validation layers to load.
 * @note The last item in this list must be NULL.
 * @param instance_info Vulkan's VkInstanceCreateInfo structure to fill in.
 */
char
vulkan_fill_in_validation_layer_info(const struct renderer_t* renderer,
                                     const char** validation_layer_names,
                                     VkInstanceCreateInfo* instance_info);

/**
 * @brief Cleans up data that was filled in to the instance info struct.
 */
void
vulkan_clean_up_validation_layer_info(VkInstanceCreateInfo* instance_info);

/**
 * @brief Attempts to load the specified list of extensions and fills in the
 * appropriate info into instance_info. If any of the specified extensions were
 * not found, then a warning is written to the log and loading that specific
 * extension is skipped without error.
 * @note vulkan_clean_up_extension_info() must be called to clean up any
 * buffers allocated when this function returns successfully.
 * @param renderer The renderer instance. Is not modified.
 * @param extension_names A list of char-arrays containing the names of the
 * extensions to load.
 * @note The last item in the list must be NULL.
 * @param instance_info Vulkan's VkInstanceCreateInfo structure to fill in.
 */
char
vulkan_fill_in_extension_info(const struct renderer_t* renderer,
                              const char** extension_names,
                              VkInstanceCreateInfo* instance_info);

/**
 * @brief Cleans up data that was filled in to the instance info struct.
 */
void
vulkan_clean_up_extension_info(VkInstanceCreateInfo* instance_info);

C_HEADER_END
