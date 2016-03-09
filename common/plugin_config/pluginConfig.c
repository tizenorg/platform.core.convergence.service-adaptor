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

/**
 * @file pluginConfig.c
 * @author Dawid Kozinski (d.kozinski@samsung.com)
 *
 * @brief Plugin configuration parser done in C99.
 */

/* threads */
#include <glib.h>
#include "pluginConfig.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

#include "pluginConfigLog.h"

#define UNUSED(x) (void)(x)

/*********************************************************************************
 *  C99 implementation
 ******************************************************************************/
/*bool a = true; */

/******************************************************************************
 *
 *  Implicit Declarations of trimming functions
 *  this only describes and assumes allocated elsewhere (trimming.c)
 *
 ******************************************************************************/
/**
* @brief trim from start, i.e. left side
* Trimming leading whitespace from string
*
* @param s ...
* @return :char*
**/
extern char *plugin_internal_ltrim(char *s);

/**
* @brief trim from end, i.e. right side
* Trimming trailing whitespace from string
*
* @param s ...
* @return :char*
**/
extern char *plugin_internal_rtrim(char *s);

/**
* @brief trim from both ends
* Trimming leading and trailing whitespace from string
* @param s ...
* @return :string&
**/
extern char *plugin_internal_trim(char *s);

/******************************************************************************
 * Typedefs
 ******************************************************************************/

/******************************************************************************
 * [unique name][cloud_adaptor_handle*]
 ******************************************************************************/
/**
* @typedef plugin_internal_MapStrStr
* Section data
* |---------------------------|-----------------------------------------------|
* | KEY : char*               | VALUE : char*                                 |
* |---------------------------|-----------------------------------------------|
* |         KEY1              |    VAL1                                       |
* |---------------------------|-----------------------------------------------|
* |         KEY2              |    VAL2                                       |
* |---------------------------|-----------------------------------------------|
* |         KEY3              |    VAL3                                       |
* |---------------------------|-----------------------------------------------|
*/
typedef GHashTable plugin_internal_MapStrStr;

/**
* @typedef plugin_internal_MapStrSection
* Sections
* |---------------------------|-----------------------------------------------|
* | SECTION NAME : char*      | SECTION PTR      : MapStrStr *                |
* |---------------------------|-----------------------------------------------|
* |         SN1               |    SP1                                        |
* |---------------------------|-----------------------------------------------|
* |         SN2               |    SP2                                        |
* |---------------------------|-----------------------------------------------|
* |         SN3               |    SP3                                        |
* |---------------------------|-----------------------------------------------|
*/
typedef GHashTable plugin_internal_MapStrSection;

/**
* @typedef plugin_internal_ListSection
* Sections
* |------------------------------------------|
* | SECTION PTR : MapStrStr *                |
* |------------------------------------------|
* |         SP1                              |
* |------------------------------------------|
* |         SP2                              |
* |------------------------------------------|
* |         SN3                              |
* |------------------------------------------|
*/
typedef GList plugin_internal_ListSection;

/******************************************************************************
 * ConfigParserState struct
 ******************************************************************************/
typedef struct plugin_internal_ConfigParserState {
	char *current_section_name;
	plugin_internal_MapStrStr *current_section;
} ConfigParserState;

/******************************************************************************
 * ConfigData class
 ******************************************************************************/
typedef struct plugin_internal_ConfigData {
	GMutex *data_mutex;

	/**
	* @brief Filepath to the read configuration file.
	**/
	char  *filepath;

	/**
	* @brief Describes layout of the data in the file.
	**/
	PluginConfigType type;

	/**
	* @brief Variable that stores the all sections of configuration,
	* meant to be accessed through the field 'configuration'.
	**/
	plugin_internal_ListSection *sections;

	/**
	* @brief Variable that stores the whole configuration.
	**/
	plugin_internal_MapStrSection *configuration;
} plugin_internal_ConfigData;

/******************************************************************************
 * Declarations (methods of ConfigData class)
 ******************************************************************************/

/******************************************************************************
 * Calbacks
 ******************************************************************************/

/**
 * @brief Function which is called when a data element of configuration map
 * being a member of plugin_internal_ConfigData struct
 * (@see plugin_internal_ConfigData) is destroyed.
 * @see plugin_internal_MapStrSection
 *
 * @param data pointer to the user data
 * @return void
 **/
static
void plugin_internal_ConfigData_cbMapStrSectionFreeData(gpointer data);

#if 0
/**
 * @brief Function passed to g_hash_table_foreach().
 * Function is called for each key/value pair.
 *
 * @param key a key
 * @param value the value corresponding to the key
 * @param user_data user data passed to g_hash_table_foreach()
 * @return void
 **/
static
void plugin_internal_ConfigData_cbMapStrSectionEntryCopy(gpointer key,
	gpointer value,
	gpointer user_data);

/**
 * @brief Function passed to g_hash_table_foreach().
 * Function is called for each key/value pair.
 *
 * @param key a key
 * @param value the value corresponding to the key
 * @param user_data user data passed to g_hash_table_foreach()
 * @return void
 **/
static
void plugin_internal_ConfigData_cbMapStrSectionCopy(gpointer key,
	gpointer value,
	gpointer user_data);


/**
 * @brief Function passed to g_list_foreach().
 * The function called for each element's data.
 *
 * @param data the element's data.
 * @param user_data user data passed to g_list_foreach() or g_slist_foreach().
 * @return gpointer
 **/
static
gpointer plugin_internal_ConfigData_cbListSectionCopy(gconstpointer data,
	gpointer user_data);
#endif

/**
 * @brief The function to be called to free each element's data.
 * The function which is called when a data element is destroyed.
 * It is passed the pointer to the data element and should free any memory and
 * resources allocated for it.
 *
 * @param data the data element.
 * @return void
 **/
static
void plugin_internal_ConfigData_cbDestroySectionsList(gpointer data);

/******************************************************************************
 * Constructing object
 ******************************************************************************/
/**
 * @brief Contruct object of type plugin_internal_ConfigData
 * @return plugin_internal_ConfigData*
 **/
/*plugin_internal_ConfigData *plugin_internal_ConfigData_new(); */

/**
* @brief Contruct object of type plugin_internal_ConfigData and loads
* configuration from the specified file.
*
* @param filepath a path to the configuration file
* @param type expected type of configuration file, this value
* determines how the file is parsed
**/
static
plugin_internal_ConfigData *
plugin_internal_ConfigData_new(const char *filepath, PluginConfigType type);

/**
* @brief Copy constructor.
*
* @param source ...
**/
#if 0
static
plugin_internal_ConfigData *
plugin_internal_ConfigData_copy(const plugin_internal_ConfigData *source);
#endif

/******************************************************************************
 * Destroying obect
 ******************************************************************************/
/**
* @brief Removes configuration from memory, rendering this configuration invalid.
*
* @return void
**/
static
void plugin_internal_ConfigData_clear(plugin_internal_ConfigData *self);

/**
* @brief Destructor.
*
**/
static
void plugin_internal_ConfigData_delete(plugin_internal_ConfigData *self);

/******************************************************************************
 * private methods
 ******************************************************************************/

/**
 * @brief parse line of INI file
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param state ...
 * @param line pointer to the string containing line of file
 * @return void
 **/
static
void plugin_internal_ConfigData_parseLineIni(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line);

/* TODO Provide implementation */
/**
 * @todo Provide implementation
 *
 * @brief parse line of GIT config file
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param state ...
 * @param line pointer to the string containing line of file
 * @return void
 **/
static
void plugin_internal_ConfigData_parseLineGit(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line);

/* TODO Provide implementation */
/**
 * @todo Provide implementation
 *
 * @brief parse line of CSV config file
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param state ...
 * @param line pointer to the string containing line of file
 * @param separator ...
 * @return void
 **/
static
void plugin_internal_ConfigData_parseLineCsv(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line,
	char separator);

/* Public */
#if 0
/**
 * @brief ...
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param section ...
 * @return int
 **/
static
bool
plugin_internal_ConfigData_hasSection(const plugin_internal_ConfigData *self,
						const char *section);


/**
* @brief This method assumes that the given section exists. Use hasSection()
* first, or use hasSectionAndKey() instead.
*
* @param self pointer to object of type plugin_internal_ConfigData
* @param section name of the configuration section
* @param key name of the key within that section
* @return int 1 if such key exists
**/
static
bool
plugin_internal_ConfigData_hasKey(const plugin_internal_ConfigData *self,
						const char *section,
						const char *key);
#endif

/**
 * @brief ...
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param section ...
 * @param key ...
 * @return int
 **/
static
bool
plugin_internal_ConfigData_hasSectionAndKey(const plugin_internal_ConfigData *self,
						const char *section,
						const char *key);

#if 0
/**
 * @brief ...
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @return const char*
 **/
static
const char *
plugin_internal_ConfigData_getFilepath(const plugin_internal_ConfigData *self);

/**
 * @brief ...
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @return PluginConfigType
 **/
static
PluginConfigType
plugin_internal_ConfigData_getType(const plugin_internal_ConfigData *self);
#endif

/**
 * @brief ...
 *
 * @param self pointer to object of type plugin_internal_ConfigData
 * @param section_name ...
 * @param section_key ...
 * @return const char*
 **/
static
const char *
plugin_internal_ConfigData_getEntry(const plugin_internal_ConfigData *self,
						const char *section_name,
						const char *section_key);

/******************************************************************************
 * Definitions (methods of ConfigData class)
 ******************************************************************************/

/******************************************************************************/
static
void plugin_internal_ConfigData_cbMapStrSectionFreeData(gpointer data)
{
	/* TODO Is it enough ? */
	/* free(data); */
	/* data = NULL; */
}

#if 0
/******************************************************************************/
plugin_internal_ConfigData *plugin_internal_ConfigData_new()
{
	plugin_internal_ConfigData *self;
	self = (plugin_internal_ConfigData *)calloc(1, sizeof(plugin_internal_ConfigData));

	GMutex *data_mutex = (GMutex *)calloc(1, sizeof(GMutex));
	g_mutex_init(data_mutex);

	self->data_mutex = data_mutex;
	self->filepath = NULL;
	self->type = CCT_INI;
	self->sections = NULL;
	self->configuration = NULL;

	self->configuration =  g_hash_table_new_full(g_str_hash,   /* Hash function  */
						g_str_equal,	/* Comparator */
						plugin_internal_ConfigData_cbMapStrSectionFreeData,   /* Key destructor */
						plugin_internal_ConfigData_cbMapStrSectionFreeData);  /* Val destructor */

	return self;
}
#endif

/******************************************************************************/
static
plugin_internal_ConfigData *
plugin_internal_ConfigData_new(const char *filepath, PluginConfigType type)
{
	startfunc;

	if (NULL == filepath) {
		return NULL;
	}

	if (CCT_INVALID == type) {
		return NULL;
	}

	plugin_internal_ConfigData *self;
	self = (plugin_internal_ConfigData *)calloc(1, sizeof(plugin_internal_ConfigData));

	if (NULL == self) {
		return NULL;
	}

	GMutex *data_mutex = (GMutex *)calloc(1, sizeof(GMutex));
	if (NULL == data_mutex) {
		free(self);
		return NULL;
	}

	g_mutex_init(data_mutex);

	self->data_mutex = data_mutex;
	self->filepath = strdup(filepath);
	self->type = CCT_INI;
	self->sections = NULL; /* sections.reserve(128); */
	self->configuration = NULL;

	self->configuration =
		g_hash_table_new_full(g_str_hash,  /* Hash function  */
						g_str_equal, /* Comparator     */
						plugin_internal_ConfigData_cbMapStrSectionFreeData, /* Key destructor */
						plugin_internal_ConfigData_cbMapStrSectionFreeData);/* Val destructor */

	FILE *fp;
	char *line;
	size_t len = 0;
	ssize_t read;

	fp = fopen(filepath, "r");

	if (NULL == fp) {
		/* exit(EXIT_FAILURE); */
		if (self) {
			if (self->data_mutex) {
				g_mutex_clear(self->data_mutex);
				free(self->data_mutex);
			}
			if (self->filepath)
				free(self->filepath);
			if (self->configuration)
				g_hash_table_destroy(self->configuration);
			free(self);
		}
		return NULL;
	}

	ConfigParserState state;
	state.current_section = NULL;

	while ((read = getline(&line, &len, fp)) != -1) {
		/* printf("Retrieved line of length %zu :\n", read); */
		/* printf("%s", line); */

		if (strlen(line) == 0)
			continue;

		switch (type) {
		case CCT_INI:
			plugin_internal_ConfigData_parseLineIni(self, &state, line);
			break;
		case CCT_GIT:
			plugin_internal_ConfigData_parseLineGit(self, &state, line);
			break;
		case CCT_CSV_COMMA:
			plugin_internal_ConfigData_parseLineCsv(self, &state, line, ',');
			break;
		case CCT_CSV_TAB:
			plugin_internal_ConfigData_parseLineCsv(self, &state, line, '\t');
			break;
		case CCT_CSV_COLON:
			plugin_internal_ConfigData_parseLineCsv(self, &state, line, ':');
			break;
		case CCT_CSV_SEMICOLON:
			plugin_internal_ConfigData_parseLineCsv(self, &state, line, ';');
			break;
		default:
			break;
		}

		free(line);
		line = NULL;
	}

/*	printf("configuration file %s was loaded\n", filepath); */
	fclose(fp);

	endfunc;
	return self;
}

#if 0
/******************************************************************************/
static
void plugin_internal_ConfigData_cbMapStrSectionEntryCopy(gpointer key,
	gpointer value,
	gpointer user_data)
{
	startfunc;

	plugin_internal_MapStrStr *copy_section = (plugin_internal_MapStrStr *)user_data;
	g_hash_table_insert(copy_section, strdup((char *)key), strdup((char *)value));

	endfunc;
}

/******************************************************************************/

static
void plugin_internal_ConfigData_cbMapStrSectionCopy(gpointer key,
	gpointer value,
	gpointer user_data)
{
	startfunc;

	plugin_internal_MapStrSection *copy_configuration = (plugin_internal_MapStrSection *)user_data;

	plugin_internal_MapStrStr *section = (plugin_internal_MapStrStr *)value;

	plugin_internal_MapStrStr *copy_section =
	g_hash_table_new_full(g_str_hash,   /* Hash function  */
				g_str_equal, /* Comparator     */
				plugin_internal_ConfigData_cbMapStrSectionFreeData,   /* Key destructor */
				plugin_internal_ConfigData_cbMapStrSectionFreeData);  /* Val destructor */

	g_hash_table_foreach(section, (GHFunc)plugin_internal_ConfigData_cbMapStrSectionEntryCopy, copy_section);

	g_hash_table_insert(copy_configuration, strdup((char *)key), copy_section);

	endfunc;
}

/******************************************************************************/
static
gpointer plugin_internal_ConfigData_cbListSectionCopy(gconstpointer src,
	gpointer data)
{
	startfunc;

	UNUSED(data);

	plugin_internal_MapStrStr *section = (plugin_internal_MapStrStr *)src;

	plugin_internal_MapStrStr *copy_section =
	g_hash_table_new_full(g_str_hash,   /* Hash function  */
				g_str_equal, /* Comparator     */
				plugin_internal_ConfigData_cbMapStrSectionFreeData,   /* Key destructor */
				plugin_internal_ConfigData_cbMapStrSectionFreeData);  /* Val destructor */

	g_hash_table_foreach(section,
				(GHFunc)plugin_internal_ConfigData_cbMapStrSectionEntryCopy,
				copy_section);

	endfunc;
	return copy_section;
}

/******************************************************************************/
#endif

#if 0
static
plugin_internal_ConfigData *
plugin_internal_ConfigData_copy(const plugin_internal_ConfigData *source)
{
	startfunc;

	if (NULL == source) {
		return NULL;
	}

	plugin_internal_ConfigData *self;
	self = (plugin_internal_ConfigData *)calloc(1, sizeof(plugin_internal_ConfigData));

	GMutex *data_mutex = (GMutex *)calloc(1, sizeof(GMutex));
	g_mutex_init(data_mutex);

	g_mutex_lock(source->data_mutex);

	self->data_mutex = data_mutex;
	self->filepath = strdup(source->filepath);
	self->type = source->type;

	self->sections = NULL;

	self->configuration =
	g_hash_table_new_full(g_str_hash,  /* Hash function  */
				g_str_equal, /* Comparator     */
				plugin_internal_ConfigData_cbMapStrSectionFreeData, /* Key destructor */
				plugin_internal_ConfigData_cbMapStrSectionFreeData);/* Val destructor */

	/* copy source->configuration to self->configuration */
	g_hash_table_foreach(source->configuration,
				(GHFunc)plugin_internal_ConfigData_cbMapStrSectionCopy,
				self->configuration);

	/* copy source->sections to self->sections */
	self->sections = g_list_copy_deep(source->sections,
				plugin_internal_ConfigData_cbListSectionCopy,
				NULL);

	g_mutex_unlock(source->data_mutex);

	endfunc;
	return NULL;
}
#endif
/******************************************************************************/
static
void plugin_internal_ConfigData_delete(plugin_internal_ConfigData *self)
{
	startfunc;

	if (self) {
		plugin_internal_ConfigData_clear(self);
	}

	endfunc;
}

/******************************************************************************
 * PRIVATE
 ******************************************************************************/
static
void
plugin_internal_ConfigData_parseLineIni(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line)
{
	startfunc;

	if (NULL == self || NULL == line) {
		return;
	}

	if (line[0] == ';')
		return;

	if (line[0] == '[') {
		line =  plugin_internal_trim((char *)line);

		char *begin = strchr((char *)line, '[');
		char *end = strrchr((char *)line, ']');

		if (NULL == end) {
			printf("Invalid section name %p\n", line);
			return;
		}

		int section_name_len = end - begin - 1;

		char *section_name = (char *)calloc(1, section_name_len + 1);
		if (NULL == section_name) {
			return;
		}

		strncpy(section_name, line + 1, section_name_len);
		section_name[section_name_len] = '\0';

		plugin_internal_MapStrStr *section =
		g_hash_table_new_full(g_str_hash,  /* Hash function  */
		g_str_equal, /* Comparator     */
		plugin_internal_ConfigData_cbMapStrSectionFreeData,   /* Key destructor */
		plugin_internal_ConfigData_cbMapStrSectionFreeData);  /* Val destructor */

		self->sections = g_list_append(self->sections, section);
		/*sections.push_back(MapStrStr()); */

		/*MapStrStr *section = &(sections[sections.size() - 1]); */
		g_hash_table_insert(self->configuration, strdup(section_name), section);

		/* configuration[section_name] = section; */

		state->current_section_name = section_name;
		state->current_section = section;
	} else {
		/* ---------------------------------------------------------------------
		* |0|1|2|3|4|5|6|7|8|
		* ---------------------------------------------------------------------
		* |K|E|Y|1|=|V|A|L|1|
		* ---------------------------------------------------------------------
		* pch = 4
		* len_key = 4
		* len_val = 4
		* len = 9
		*/
		line =  plugin_internal_trim((char *)line);
		const char *pch = strchr((char *)line, '=');

		if (pch != NULL) {
			int len = strlen(line);
			int key_len = pch - line + 1 ; /* +'\0' */
			int value_len = len - key_len + 1; /*  +'\0' */

			char *key = (char *)calloc(1, key_len);
			char *value = (char *)calloc(1, value_len);

			if (NULL != key && NULL != value) {
				strncpy(key, line, key_len - 1);
				key[key_len - 1] = '\0';

				strncpy(value, pch + 1, value_len - 1);
				value[value_len - 1] = '\0';

				g_hash_table_insert(state->current_section, strdup(key), strdup(value));
			}
			if (NULL != key) {
				free(key);
				key = NULL;
			}
			if (NULL != value) {
				free(value);
				value = NULL;
			}
		}
	}

	endfunc;
}

/******************************************************************************/
static
void
plugin_internal_ConfigData_parseLineGit(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line)
{
	startfunc;

	if (NULL == self) {
		return;
	}

	UNUSED(state);
	UNUSED(line);

	endfunc;
}

/******************************************************************************/
static
void
plugin_internal_ConfigData_parseLineCsv(plugin_internal_ConfigData *self,
	ConfigParserState *state,
	const char *line,
	char separator)
{
	startfunc;

	if (NULL == self) {
		return;
	}

	UNUSED(state);
	UNUSED(line);
	UNUSED(separator);

	endfunc;
}

/******************************************************************************/
static
void plugin_internal_ConfigData_cbDestroySectionsList(gpointer data)
{
	startfunc;

	plugin_internal_MapStrStr *section = (plugin_internal_MapStrStr *)data;
	g_hash_table_destroy(section);

	endfunc;
}

/******************************************************************************/
static
void plugin_internal_ConfigData_clear(plugin_internal_ConfigData *self)
{
	startfunc;

	if (NULL == self) {
		return;
	}

	free(self->filepath);

	self->type = CCT_INVALID;

	g_hash_table_destroy(self->configuration);
	self->configuration = NULL;
	g_list_free_full(self->sections, plugin_internal_ConfigData_cbDestroySectionsList);

	endfunc;
}
#if 0
/******************************************************************************/
static
bool
plugin_internal_ConfigData_hasKey(const plugin_internal_ConfigData *self,
						const char *section_name,
						const char *section_key)
{
	startfunc;

	if (NULL == self || NULL == self->configuration) {
		return false;
	}

	g_mutex_lock(self->data_mutex);

	gconstpointer lookup_key = (gconstpointer)section_name;
	gpointer orig_key = NULL; /* key of self->configuration */
	gpointer value = NULL;    /* value of self->configuration */

	gboolean result = g_hash_table_lookup_extended(self->configuration,
						lookup_key,
						&orig_key,
						&value);

	if (TRUE == result && value) {
		plugin_internal_MapStrStr *ptrMapSection = (plugin_internal_MapStrStr *)value ;

		lookup_key = (gconstpointer)section_key;
		orig_key = NULL; /* key of ptrMapSection */
		value = NULL;    /* value of ptrMapSection */

		result = g_hash_table_lookup_extended(ptrMapSection,
						lookup_key,
						&orig_key,
						&value);
	}

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return result;
}
#endif

/******************************************************************************
 * PUBLIC
 ******************************************************************************/
#if 0
static
bool
plugin_internal_ConfigData_hasSection(const plugin_internal_ConfigData *self,
						const char *section_name)
{
	startfunc;

	if (NULL == self) {
		return false;
	}

	g_mutex_lock(self->data_mutex);

	gconstpointer lookup_key = (gconstpointer)section_name;
	gpointer orig_key = NULL; /* key of self->configuration */
	gpointer value = NULL;    /* value of self->configuration */

	gboolean result = g_hash_table_lookup_extended(self->configuration,
						lookup_key,
						&orig_key,
						&value);

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return result;
}
#endif
/******************************************************************************/
static
bool
plugin_internal_ConfigData_hasSectionAndKey(const plugin_internal_ConfigData *self,
						const char *section_name,
						const char *section_key)
{
	startfunc;

	if (NULL == self) {
		return false;
	}

	g_mutex_lock(self->data_mutex);

	gconstpointer lookup_key = (gconstpointer)section_name;
	gpointer orig_key = NULL; /* key of self->configuration */
	gpointer value = NULL;    /* value of self->configuration */

	gboolean result = g_hash_table_lookup_extended(self->configuration,
						lookup_key,
						&orig_key,
						&value);

	if (TRUE == result && value) {
		plugin_internal_MapStrStr *ptrMapSection = (plugin_internal_MapStrStr *)value ;
						lookup_key = (gconstpointer)section_key;
						orig_key = NULL; /* key of ptrMapSection */
						value = NULL;    /* value of ptrMapSection */

		result = g_hash_table_lookup_extended(ptrMapSection,
						lookup_key,
						&orig_key,
						&value);
	}

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return result;
}

#if 0
/******************************************************************************/
static
const char *
plugin_internal_ConfigData_getFilepath(const plugin_internal_ConfigData *self)
{
	startfunc;

	if (NULL == self) {
		return NULL;
	}

	g_mutex_lock(self->data_mutex);

	const char  *result = self->filepath;

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return result;
}

/******************************************************************************/
static
PluginConfigType
plugin_internal_ConfigData_getType(const plugin_internal_ConfigData *self)
{
	startfunc;

	if (NULL == self) {
		return CCT_INVALID;
	}

	g_mutex_lock(self->data_mutex);

	PluginConfigType result = self->type;

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return result;
}
#endif

/******************************************************************************/
static
const char *
plugin_internal_ConfigData_getEntry(const plugin_internal_ConfigData *self,
						const char *section_name,
						const char *section_key)
{
	startfunc;

	if (NULL == self) {
		return NULL;
	}

	g_mutex_lock(self->data_mutex);

	gconstpointer lookup_key = (gconstpointer)section_name;
	gpointer orig_key = NULL; /* key of self->configuration */
	gpointer value = NULL;    /* value of self->configuration */

	gboolean result = g_hash_table_lookup_extended(self->configuration,
						lookup_key,
						&orig_key,
						&value);

	if (TRUE == result && value) {
		plugin_internal_MapStrStr *ptrMapSection = (plugin_internal_MapStrStr *)value ;

		lookup_key = (gconstpointer)section_key;
		orig_key = NULL; /* key of ptrMapSection */
		value = NULL;    /* value of ptrMapSection */

		result = g_hash_table_lookup_extended(ptrMapSection,
						lookup_key,
						&orig_key,
						&value);
	}

	g_mutex_unlock(self->data_mutex);

	endfunc;
	return (char *)value;
}

/******************************************************************************
 * Config class
 ******************************************************************************/
/**
 * @brief General-purpose, class for loading and reading configuration from
 * simple plaintext files.
 **/
typedef struct plugin_Config {
	bool _loaded;
	plugin_internal_ConfigData *_configuration;
} plugin_Config;

/******************************************************************************
 * Declarations (methods of Config class)
 ******************************************************************************/

/**
 * @brief Construct object of type plugin_Config
 *
 * @return plugin_Config*
 **/
static
plugin_Config *plugin_Config_new();

/**
 * @brief Destroy object of type plugin_Config
 *
 * @param self pointer to object of type plugin_Config
 * @return void
 **/
static
void plugin_Config_delete(plugin_Config *self);

/**
 * @brief Loads configuration to memory and/or gets raw data from configuration.
 *
 * @param self pointer to object of type plugin_Config
 * @param section will return value attached to a key from this section
 * @param key will return value attached to this key
 * @return char*
 **/
static
const char *
plugin_Config_getRaw(plugin_Config *self, const char *section, const char *key);

/**
 * @brief Loads the configuration from a given file into memory.
 *
 * @param self pointer to object of type plugin_Config
 * @param filepath path to the configuration file that will be loaded
 * @param type expected type of configuration file, this value determines
 * how the file is parsed
 * @return int return 1 if successful, 0 otherwise
 */
static
int
plugin_Config_load(plugin_Config *self, const char *filepath, PluginConfigType type);

/**
 * @brief Unloads the configuration from the memory.
 *
 * The configuration is automatically unloaded when the Config object is
 * destoyed, so this method does not have to be executed unless you need
 * to extremely lower memory usage and few bytes matter.
 *
 * @param self pointer to object of type plugin_Config
 * @return void
 **/
static void plugin_Config_unload(plugin_Config *self);

/**
* @brief Checks wheteher config file has been loaded.
*
* @return int return 1 if successful, 0 otherwise
**/
static bool plugin_Config_isLoaded(plugin_Config *self);

/**
 * @brief From loaded configuration, gets string value attached to given key.
 *
 * @param self pointer to object of type plugin_Config
 * @param section ...
 * @param key ...
 * @return char* value attached to the key from the section
 **/
static
const char *
plugin_Config_getString(plugin_Config *self, const char *section, const char *key);

/**
 * @brief From loaded configuration, gets integer value attached to given key.
 *
 * @param self pointer to object of type plugin_Config
 * @param section will return value attached to a key from this section
 * @param key will return value attached to this key
 * @return int
 **/
static
int plugin_Config_getInt(plugin_Config *self, const char *section, const char *key);

#if 0
/**
 * @brief From _loaded configuration, gets double value attached to given key.
 *
 * @param self pointer to object of type plugin_Config
 * @param section will return value attached to a key from this section
 * @param key will return value attached to this key
 * @return double
 **/
static
double getDouble(plugin_Config *self, const char *section, const char *key);
#endif

/******************************************************************************
 * Definitiions (methods of Config class)
 ******************************************************************************/
static
plugin_Config *plugin_Config_new()
{
	startfunc;

	plugin_Config *self;
	self = (plugin_Config *)calloc(1, sizeof(plugin_Config));

	if (NULL == self) {
		return NULL;
	}

	self->_loaded = false;
	self->_configuration = NULL;

	endfunc;
	return self;
}

/******************************************************************************/
static
void plugin_Config_delete(plugin_Config *self)
{
	startfunc;

	if (self && self->_configuration) {
		free(self->_configuration);
		self->_configuration = NULL;
	} else {
		plugin_config_error("Invalid parameter");
	}
	free(self);

	endfunc;
}

/******************************************************************************/
static
const char *
plugin_Config_getRaw(plugin_Config *self, const char *section, const char *key)
{
	startfunc;

	const char *ret = NULL;

	if (self && self->_configuration) {
		if (plugin_internal_ConfigData_hasSectionAndKey(
			(const plugin_internal_ConfigData *)self->_configuration, section, key)) {
			ret = plugin_internal_ConfigData_getEntry((const plugin_internal_ConfigData *)self->_configuration,
									section,
									key);
		}
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}

/******************************************************************************/
static
int plugin_Config_load(plugin_Config *self, const char *filepath, PluginConfigType type)
{
	startfunc;
	int ret = 0;

	if (self) {
		if (self->_configuration) {
			plugin_internal_ConfigData_delete(self->_configuration);
			self->_configuration = NULL;
		}

		if (filepath && strlen(filepath) > 0) {
			self->_configuration = plugin_internal_ConfigData_new(filepath, type);

			if (self->_configuration) {
				self->_loaded = 1;
				ret = 1;
			}
		}
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}

/******************************************************************************/
static
void plugin_Config_unload(plugin_Config *self)
{
	startfunc;

	if (self && self->_configuration) {
		plugin_internal_ConfigData_delete(self->_configuration);
		self->_configuration = NULL;
		self->_loaded = false;
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
}

/******************************************************************************/
static
bool plugin_Config_isLoaded(plugin_Config *self)
{
	startfunc;

	bool ret = false;

	if (self) {
		ret = self->_loaded;
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}

/******************************************************************************/
static
const char *
plugin_Config_getString(plugin_Config *self, const char *section, const char *key)
{
	plugin_config_debug_func();

	return plugin_Config_getRaw(self, section, key);
}


/******************************************************************************/
static
int plugin_Config_getInt(plugin_Config *self, const char *section, const char *key)
{
	plugin_config_debug_func();

	const char *result = plugin_Config_getRaw(self, section, key);
	if (NULL == result) {
		return INT_MIN;
	}

	return atoi(result);

}

/******************************************************************************/
static
double plugin_Config_getDouble(plugin_Config *self, const char *section, const char *key)
{
	plugin_config_debug_func();

	const char *result = plugin_Config_getRaw(self, section, key);
	if (NULL == result) {
		return DBL_MIN;
	}

	return atof(result);
}

/******************************************************************************
 * C API implementation
 ******************************************************************************/

/******************************************************************************/
ConfigHandle plugin_config_create()
{
	plugin_config_debug_func();

	return (ConfigHandle)plugin_Config_new();
}

/******************************************************************************/
void plugin_config_delete(ConfigHandle config_handle)
{
	startfunc;

	plugin_Config_delete((plugin_Config *)config_handle);

	endfunc;
}

/******************************************************************************/
void plugin_config_load(ConfigHandle config_handle, const char *filepath,
						PluginConfigType type)
{
	startfunc;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		plugin_Config_load(pConfig, filepath, type);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
}

/******************************************************************************/
void plugin_config_unload(ConfigHandle config_handle)
{
	startfunc;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		plugin_Config_unload(pConfig);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
}

/******************************************************************************/
const char *plugin_config_get_string(ConfigHandle config_handle,
						const char *section, const char *key)
{
	startfunc;

	const char *val = NULL;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		val =  plugin_Config_getString(pConfig, section, key);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return val;
}

/******************************************************************************/
int plugin_config_get_int(ConfigHandle config_handle, const char *section, const char *key)
{
	startfunc;

	int ret = 0;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		ret = plugin_Config_getInt(pConfig, section, key);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}

/******************************************************************************/
double plugin_config_get_double(ConfigHandle config_handle, const char *section, const char *key)
{
	startfunc;

	double ret = 0;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		ret = plugin_Config_getDouble(pConfig, section, key);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}

/******************************************************************************/
bool plugin_config_is_loaded(ConfigHandle config_handle)
{
	startfunc;
	bool ret = false;

	if (config_handle) {
		plugin_Config *pConfig = (plugin_Config *)(config_handle);
		ret =  plugin_Config_isLoaded(pConfig);
	} else {
		plugin_config_error("Invalid parameter");
	}

	endfunc;
	return ret;
}
