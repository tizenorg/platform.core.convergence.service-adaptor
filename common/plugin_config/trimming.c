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
 * @file trimming.c
 * @author Dawid Kozinski (d.kozinski@samsung.com)
 *
 * @brief Trimming functions taken from
 * http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

/**
* @brief trim from start, i.e. left side
* Trimming leading whitespace from string
*
* @param s ...
* @return :char*
**/
char *plugin_internal_ltrim(char *s);

/**
* @brief trim from end, i.e. right side
* Trimming trailing whitespace from string
*
* @param s ...
* @return :char*
**/
char *plugin_internal_rtrim(char *s);

/**
* @brief trim from both ends
* Trimming leading and trailing whitespace from string
* @param s ...
* @return :string&
**/
char *plugin_internal_trim(char *s);

/* ////////////////////////////////////////////////////////////////////////////////
   // Implementation
   //////////////////////////////////////////////////////////////////////////////// */
char *plugin_internal_ltrim(char *s)
{
	char *newstart = s;

	while (isspace(*newstart)) {
		++newstart;
	}

	/* newstart points to first non-whitespace char (which might be '\0') */
	memmove(s, newstart, strlen(newstart) + 1);   /* don't forget to move the '\0' terminator */

	return s;
}

char *plugin_internal_rtrim(char *s)
{
	char *end = s + strlen(s);

	/* find the last non-whitespace character */
	while ((end != s) && isspace(*(end - 1))) {
		--end;
	}

	/* at this point either (end == s) and s is either empty or all whitespace
		so it needs to be made empty, or
		end points just past the last non-whitespace character (it might point
		at the '\0' terminator, in which case there's no problem writing
		another there). */
	*end = '\0';

	return s;
}

char *plugin_internal_trim(char *s)
{
	return plugin_internal_rtrim(plugin_internal_ltrim(s));
}
