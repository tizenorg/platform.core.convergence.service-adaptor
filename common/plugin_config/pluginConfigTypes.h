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

#ifndef PLUGIN_CONFIG_TYPES
#define PLUGIN_CONFIG_TYPES

/**
 * @file pluginConfigTypes.h
 * @author Dawid Kozinski (d.kozinski@samsung.com)
 *
 * @brief Plugin configuration parser done in C++.
 */

/**
 * @brief Types of supported configuration files.
 **/
typedef enum PluginConfigType {
	/**
	* @brief INI file format, as defined here: https://en.wikipedia.org/wiki/INI_file#Format
	**/
	CCT_INI = 1,

	/**
	* @brief Git config file format, as defined here:
	**/
	CCT_GIT = 1 << 1,

	/**
	* @brief Three column CSV file, columns are: section, key, value.
	* They are separated with commas.
	**/
	CCT_CSV_COMMA = 1 << 2,

	/**
	* @brief Three column CSV file, columns are: section, key, value.
	* They are separated with tabs.
	**/
	CCT_CSV_TAB = 1 << 3,

	/**
	* @brief Three column CSV file, columns are: section, key, value.
	* They are separated with colons.
	**/
	CCT_CSV_COLON = 1 << 4,

	/**
	* @brief Three column CSV file, columns are: section, key, value.
	* They are separated with semicolons.
	**/
	CCT_CSV_SEMICOLON = 1 << 5,

	/**
	* @brief As the name suggessts.
	**/
	CCT_INVALID = 0
} PluginConfigType;

#endif /* PLUGIN_CONFIG_TYPES */
