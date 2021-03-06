/*!
 * @file yaml.h
 */

#include <stdio.h>
#include "util/pstdint.h"
#include "util/config.h"
#include "util/unordered_vector.h"

C_HEADER_BEGIN

struct ptree_t;

/*!
 * @brief Initialises the yaml parser. This must be called before using any
 * other yaml-related functions.
 */
UTIL_PUBLIC_API void
yaml_init(void);

/*!
 * @brief De-initialises the yaml parser and cleans up any nodes that weren't
 * closed.
 */
UTIL_PUBLIC_API void
yaml_deinit(void);

UTIL_PUBLIC_API struct ptree_t*
yaml_create(void);

/*!
 * @brief Loads and parses a yaml file.
 * @param filename The file to load.
 * @return Returns a new yaml node object if successful. If a parser error
 * occurs, or if the file doesn't exist, NULL is returned.
 */
UTIL_PUBLIC_API struct ptree_t*
yaml_load(const char* filename);

#if defined(UTIL_PLATFORM_LINUX) || defined(LIGHTSHIP_UTIL_PLATFORM_MACOSX)
/*!
 * @brief Parses a string containing YAML code located in memory.
 * @param buffer The string to load YAML from.
 * @return Returns a new yaml node object if successful. If a parser error
 * occurs, NULL is returned.
 */
UTIL_PUBLIC_API struct ptree_t*
yaml_load_from_memory(const char* buffer);
#endif

/*!
 * @brief Parses YAML code from a stream.
 * @param stream The stream to load YAML from.
 * @return Returns a new yaml node object if successful. If a parser error
 * occurs, or if the stream isn't open, NULL is returned.
 */
UTIL_PUBLIC_API struct ptree_t*
yaml_load_from_stream(FILE* stream);

/*!
 * @brief Destroys a loaded yaml nodeument.
 * @param node The nodeument to destroy.
 */
UTIL_PUBLIC_API void
yaml_destroy(struct ptree_t* node);

/*!
 * @brief Looks up the specified node from a loaded yaml nodeument and retrieves
 * its value.
 *
 * For example, if your YAML file looked like this:
 * ```
 * root:
 *     my_items:
 *         item1: value1
 *         item2: value2
 * ```
 * And you wanted to retrieve *value2*, you would write:
 * ```const char* value = yaml_get_value(node, "root.my_items.item1");```
 * @param node The nodeument to search in.
 * @param key The key(s) to search for.
 * @return Returns the value if it was successfully found, otherwise NULL is
 * returned.
 */
UTIL_PUBLIC_API const char*
yaml_get_value(const struct ptree_t* node, const char* key);

/*!
 * @brief Looks up the specified node from a loaded yaml file and returns it.
 *
 * For example, if your YAML file looked like this:
 * ```
 * root:
 *     my_items:
 *         item1: value1
 *         item2: value2
 * ```
 * And you wanted to retrieve the node *item1*, you would write:
 * ```struct ptree_t* item1 = yaml_get_node(node, "root.my_items.item1");```
 * @param node The nodeument to search in.
 * @param key The key(s) to search for.
 * @return Returns a property-tree node if the node was successfully found,
 * otherwise NULL is returned.
 */
UTIL_PUBLIC_API struct ptree_t*
yaml_get_node(const struct ptree_t* node, const char* key);

UTIL_PUBLIC_API struct ptree_t*
yaml_set_value(struct ptree_t* node, const char* key, const char* value);

UTIL_PUBLIC_API char
yaml_remove_value(struct ptree_t* node, const char* key);

UTIL_PUBLIC_API char
yaml_string_to_bool(const char* str);

#define YAML_FOR_EACH(m_root, m_key, m_hash_var, m_node_var) {               \
    struct ptree_t* yaml_internal_##m_root_node;                             \
    if((yaml_internal_##m_root_node = yaml_get_node(m_root, m_key))) {       \
        BSTHV_FOR_EACH(&(yaml_internal_##m_root_node)->children,             \
                     struct ptree_t,                                         \
                     m_hash_var,                                             \
                     m_node_var)

#define YAML_END_EACH BSTHV_END_EACH }}

#define YAML_IF_EACH_FAILED BSTHV_END_EACH } else {

#define YAML_ENDIF_EACH_FAILED }}

C_HEADER_END
