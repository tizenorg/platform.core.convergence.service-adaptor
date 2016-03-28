/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the License);
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef PLUGIN_CONFIG_H_INCLUDED
#define PLUGIN_CONFIG_H_INCLUDED

#ifndef API
#define API __attribute__ ((visibility("default")))
#endif

#include <stdbool.h>

/**
 * @file pluginConfig.h
 * @author Dawid Kozinski (d.kozinski@samsung.com)
 *
 * @brief Plugin configuration parser done in C++.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pluginConfigTypes.h"

/**
* @brief ...
**/
typedef void *ConfigHandle;

/**
* @brief General-purpose, class for loading and reading configuration from
* simple plaintext files.
**/

/**
* @brief Creates a new empty Config object and returns an opaque "handle"
*
* @return ConfigHandle an opaque pointer to Config object.
**/
API
ConfigHandle plugin_config_create();

/**
* @brief Destroy config object
*
* @param config_handle an opaque pointer to Config object
* @return void
**/
API
void plugin_config_delete(ConfigHandle config_handle);

/**
* @brief Loads the configuration from given file to memory.
*
* @param config_handle an opaque pointer to Config object
* @param filepath configuration will be _loaded from given filepath
* @param type expected type of configuration file, this value determines how
* the file is parsed
* @return void
**/
API
void plugin_config_load(ConfigHandle config_handle, const char *filepath, PluginConfigType type);

/**
* @brief Unloads the configuration from the memory.
*
* The configuration is automatically un_loaded when the program is exitting,
* so this method does not have to be executed unless you need to extremely
* lower memory usage and few bytes matter.
*
* @param config_handle an opaque pointer to Config object
* @return void
**/
API
void plugin_config_unload(ConfigHandle config_handle);

/**
* @brief From _loaded configuration, gets string value attached to given key.
*
* @param config_handle an opaque pointer to Config object
* @param section will return value attached to a key from this section
* @param key will return value attached to this key
* @return :string
**/
API
const char *plugin_config_get_string(ConfigHandle config_handle, const char *section, const char *key);

/**
* @brief From _loaded configuration, gets integer value attached to given key.
*
* @param config_handle an opaque pointer to Config object
* @param section will return value attached to a key from this section
* @param key will return value attached to this key
* @return int
**/
API
int plugin_config_get_int(ConfigHandle config_handle, const char *section, const char *key);

/**
* @brief From _loaded configuration, gets double value attached to given key.
*
* @param config_handle an opaque pointer to Config object
* @param section will return value attached to a key from this section
* @param key will return value attached to this key
* @return double
**/
API
double plugin_config_get_double(ConfigHandle config_handle, const char *section, const char *key);

/**
* @brief Checks wheteher config file has been loaded.
*
* @param config_handle an opaque pointer to Config object
* @return 0 if config is not loaded
*         1 if config is loaded
**/
API
bool plugin_config_is_loaded(ConfigHandle config_handle);

#ifdef __cplusplus
}
#endif
#endif /* PLUGIN_CONFIG_H_INCLUDED */

