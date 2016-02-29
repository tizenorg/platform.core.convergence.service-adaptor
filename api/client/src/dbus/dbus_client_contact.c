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

/******************************************************************************
 * File: dbus-client-contact.c
 * Desc:
 * Created on: Oct, 2014
 * Auth: Yongjin Kim <youth.kim@samsung.com>
 *
 *****************************************************************************/

#include <gio/gio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include <dbus-server.h>

#include "dbus_client.h"
#include "dbus_client_contact.h"
#include "service_adaptor_client_type.h"
#include "service_adaptor_client_log.h"
#include "private/service-adaptor-client-contact.h"

#include "util/service_adaptor_client_util.h"
/******************************************************************************
 * Global variables and defines
 ******************************************************************************/

/******************************************************************************
 * Private interface
 ******************************************************************************/

static void __safe_g_variant_builder_add_contact_string(GVariantBuilder *builder,
						const char *data)
{
	if (NULL == data) {
		g_variant_builder_add(builder, "s", "");
	} else {
		char *_data = g_strconcat(" ", data, NULL);
		g_variant_builder_add(builder, "s", _data);
		free(_data);
	}
}

static char *__safe_dup_contact_string(char *data)
{
	char *str = __safe_add_string(data);
	char *ret = NULL;
	if (0 < strlen(str)) {
		ret = g_strconcat(" ", str, NULL);
	} else {
		ret = strdup(str);
	}

	return ret;
}

/******************************************************************************
 * Private interface definition
 ******************************************************************************/

GVariant *__create_contact_req_type(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data)
{
	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_contact_info_req_list_type));

	for (int i = 0; i < contact_req->cts_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_contact_info_req_s_type));
		__safe_g_variant_builder_add_string(builder, contact_req->cts[i]->tp);
		__safe_g_variant_builder_add_string(builder, contact_req->cts[i]->id);
		__safe_g_variant_builder_add_string(builder, contact_req->cts[i]->pn);
		__safe_g_variant_builder_add_contact_string(builder, contact_req->cts[i]->nm);
		__safe_g_variant_builder_add_string(builder, contact_req->cts[i]->cc);
		g_variant_builder_close(builder);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_contact_req_s_type ")", __safe_add_string(service_name), contact_req->tt, builder, contact_req->cts_len);

	g_variant_builder_unref(builder);

	return request;
}

void __get_contact_res_type(GVariant *call_result_struct,
						service_adaptor_contact_res_s **contact_res,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_contact_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_contact_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	*contact_res = (service_adaptor_contact_res_s *) calloc(1, sizeof(service_adaptor_contact_res_s));

	if (NULL != (*contact_res)) {
		(*contact_res)->tt = g_variant_get_int64(res_info_struct[idx++]);
		sac_info("tt : %lld", (*contact_res)->tt);

		/* cts list */
		gsize cts_count = g_variant_n_children(res_info_struct[idx]);
		sac_info("cts count : %d", (int)cts_count);

		if (0 == cts_count) {
			(*contact_res)->cts_len = 0;
			sac_info("cts len : %u", (*contact_res)->cts_len);

			for (size_t j = 0; j < private_service_adaptor_contact_res_s_type_length; j++) {
				g_variant_unref(res_info_struct[j]);
			}
			return;
		}

		(*contact_res)->cts = (service_adaptor_contact_info_res_s **) calloc(cts_count, sizeof(service_adaptor_contact_info_res_s*));

		if (NULL == ((*contact_res)->cts)) {
			sac_error("Critical : Memory allocation failed");
			(*contact_res)->cts_len = 0;

			for (size_t j = 0; j < private_service_adaptor_contact_res_s_type_length; j++) {
				g_variant_unref(res_info_struct[j]);
			}
			return;
		}
		for (gsize i = 0; i < cts_count; i++) {
			GVariant *cts_info_struct[private_service_adaptor_contact_info_res_s_type_length];
			GVariant *cts_info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);

			(*contact_res)->cts[i] = (service_adaptor_contact_info_res_s *) calloc(1, sizeof(service_adaptor_contact_info_res_s));

			if (NULL != ((*contact_res)->cts[i])) {
				for (size_t j = 0; j < private_service_adaptor_contact_info_res_s_type_length; j++) {
					cts_info_struct[j] = g_variant_get_child_value(cts_info_entry_v, j);
				}

				int idx2 = 0;
				(*contact_res)->cts[i]->duid = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->id = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->msisdn = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->ty = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->cc = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->pn = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->nm = ipc_g_variant_dup_string(cts_info_struct[idx2++]);

				/* evnt list */
				gsize evnt_count = g_variant_n_children(cts_info_struct[idx2]);
				(*contact_res)->cts[i]->evnt = (char **) calloc(evnt_count, sizeof(char *));

				if (NULL != ((*contact_res)->cts[i]->evnt)) {
					for (gsize k = 0; k < evnt_count; k++) {
						GVariant *evnt_info_struct;
						GVariant *evnt_info_entry_v = g_variant_get_child_value(cts_info_struct[idx2], k);
						evnt_info_struct = g_variant_get_child_value(evnt_info_entry_v, 0);

						(*contact_res)->cts[i]->evnt[k] = ipc_g_variant_dup_string(evnt_info_struct);

						g_variant_unref(evnt_info_struct);
					}
				}
				idx2++;
				/* evnt list */

				(*contact_res)->cts[i]->evnt_len = g_variant_get_uint32(cts_info_struct[idx2++]);

				/* image list */
				gsize img_count = g_variant_n_children(cts_info_struct[idx2]);
				sac_info("img_count : %d", (int)img_count);
				(*contact_res)->cts[i]->images = (service_adaptor_profile_image_h *) calloc(img_count, sizeof(service_adaptor_profile_image_h));

				if (NULL != ((*contact_res)->cts[i]->images)) {
					for (gsize k = 0; k < img_count; k++) {
						(*contact_res)->cts[i]->images[k] = (service_adaptor_profile_image_h) calloc(1, sizeof(struct _service_adaptor_profile_image_s));
						if (NULL != (*contact_res)->cts[i]->images[k]) {
							GVariant *img_info_struct[2];
							GVariant *img_info_entry_v = g_variant_get_child_value(cts_info_struct[idx2], k);
							img_info_struct[0] = g_variant_get_child_value(img_info_entry_v, 0);

							img_info_struct[1] = g_variant_get_child_value(img_info_entry_v, 1);

							(*contact_res)->cts[i]->images[k]->index = g_variant_get_int32(img_info_struct[0]);
							sac_info("Test (image index : %d)", (*contact_res)->cts[i]->images[k]->index);
							char *path = ipc_g_variant_dup_string(img_info_struct[1]);
							sac_info("Test (path : %s)", path);

							if (NULL != path) {
								strncpy((*contact_res)->cts[i]->images[k]->path, path, (CONTACT_PROFILE_IMAGE_PATH_LEN - 1));
							}

							g_variant_unref(img_info_struct[0]);
							g_variant_unref(img_info_struct[1]);
						}
					}
				}
				idx2++;
				/* image list */

				(*contact_res)->cts[i]->images_len = g_variant_get_uint32(cts_info_struct[idx2++]);


				/*(*contact_res)->cts[i]->img = ipc_g_variant_dup_string(cts_info_struct[idx2++]);*/

				/* adrs list */
				gsize adrs_count = g_variant_n_children(cts_info_struct[idx2]);
				(*contact_res)->cts[i]->adrs = (char **) calloc(adrs_count, sizeof(char *));

				if (NULL != ((*contact_res)->cts[i]->adrs)) {
					for (gsize k = 0; k < adrs_count; k++) {
						GVariant *adrs_info_struct;
						GVariant *adrs_info_entry_v = g_variant_get_child_value(cts_info_struct[idx2], k);
						adrs_info_struct = g_variant_get_child_value(adrs_info_entry_v, 0);

						(*contact_res)->cts[i]->adrs[k] = ipc_g_variant_dup_string(adrs_info_struct);

						g_variant_unref(adrs_info_struct);
					}
				}
				idx2++;
				/* adrs list */

				(*contact_res)->cts[i]->adrs_len = g_variant_get_uint32(cts_info_struct[idx2++]);

				/* mail list */
				gsize mail_count = g_variant_n_children(cts_info_struct[idx2]);
				(*contact_res)->cts[i]->mail = (char **) calloc(mail_count, sizeof(char *));

				if (NULL != ((*contact_res)->cts[i]->mail)) {
					for (gsize k = 0; k < mail_count; k++) {
						GVariant *mail_info_struct;
						GVariant *mail_info_entry_v = g_variant_get_child_value(cts_info_struct[idx2], k);
						mail_info_struct = g_variant_get_child_value(mail_info_entry_v, 0);

						(*contact_res)->cts[i]->mail[k] = ipc_g_variant_dup_string(mail_info_struct);

						g_variant_unref(mail_info_struct);
					}
				}
				idx2++;
				/* mail list */

				(*contact_res)->cts[i]->mail_len = g_variant_get_uint32(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->org = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->prsc = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->status = ipc_g_variant_dup_string(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->sids = g_variant_get_uint32(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->profile_type = g_variant_get_int32(cts_info_struct[idx2++]);
				(*contact_res)->cts[i]->profile_url = ipc_g_variant_dup_string(cts_info_struct[idx2++]);

				for (size_t j = 0; j < private_service_adaptor_contact_info_res_s_type_length; j++) {
					g_variant_unref(cts_info_struct[j]);
				}
			}
		}
		idx++;
		/* cts list */

		(*contact_res)->cts_len = g_variant_get_uint32(res_info_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_contact_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

GVariant *__create_profile_req_type(const char *service_name,
						service_adaptor_profile_req_s *profile_req,
						void *user_data)
{
	GVariantBuilder *evnt_builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

	for (gsize j = 0; j < profile_req->evnt_len; j++) {
		__safe_g_variant_builder_add_array_string(evnt_builder, profile_req->evnt[j]);
	}

	GVariantBuilder *adrs_builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

	for (gsize j = 0; j < profile_req->adrs_len; j++) {
		__safe_g_variant_builder_add_array_string(adrs_builder, profile_req->adrs[j]);
	}

	GVariantBuilder *mail_builder = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

	for (gsize j = 0; j < profile_req->mail_len; j++) {
		__safe_g_variant_builder_add_array_string(mail_builder, profile_req->mail[j]);
	}

	char *converted_nm = __safe_dup_contact_string(profile_req->nm);
	GVariant *request = g_variant_new("(" private_service_adaptor_profile_req_s_type ")", __safe_add_string(service_name),
			__safe_add_string(profile_req->cc), __safe_add_string(profile_req->pn), converted_nm,
			evnt_builder, profile_req->evnt_len,
			__safe_add_string(profile_req->img),
			adrs_builder, profile_req->adrs_len, mail_builder, profile_req->mail_len,
			__safe_add_string(profile_req->org), __safe_add_string(profile_req->prsc), __safe_add_string(profile_req->status));
	free(converted_nm);

	g_variant_builder_unref(evnt_builder);
	g_variant_builder_unref(adrs_builder);
	g_variant_builder_unref(mail_builder);

	return request;
}

void __get_profile_res_type(GVariant *call_result_struct,
						service_adaptor_profile_res_s **profile_res,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_profile_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_profile_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	*profile_res = (service_adaptor_profile_res_s *) calloc(1, sizeof(service_adaptor_profile_res_s));
	if (NULL != *profile_res) {
		(*profile_res)->nm = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*profile_res)->img = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*profile_res)->prsc = ipc_g_variant_dup_string(res_info_struct[idx++]);
		(*profile_res)->status = ipc_g_variant_dup_string(res_info_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_profile_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

GVariant *__create_del_me_profile_req_type(const char *service_name,
						void *user_data)
{

	GVariant *request = g_variant_new("(" private_service_adaptor_essential_s_type ")", __safe_add_string(service_name));


	return request;
}


GVariant *__create_file_path_req_type(const char *service_name,
						service_adaptor_profile_image_h *images,
						unsigned int images_len,
						void *user_data)
{


	if (NULL == images) {
		images_len = 0;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(iis)"));

	for (int j = 0; j < images_len; j++) {
		sac_debug_func("[DBG] Set me profile image meta Params %dth [DBG]", j);
		sac_debug_func("type (%d) index (%d) path (%s)", images[j]->type, images[j]->index, images[j]->path);
		g_variant_builder_add(builder, "(iis)", (int32_t)(images[j]->type), (int32_t)images[j]->index, __safe_add_string(images[j]->path));
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_contact_profile_image_req_s_type ")", __safe_add_string(service_name),  builder, images_len);

	g_variant_builder_unref(builder);

	return request;
}

void __get_failed_image_meta_res_type(GVariant *call_result_struct,
						service_adaptor_profile_image_h **images,
						unsigned int *images_len)
{
	GVariant *res_info_struct[private_service_adaptor_contact_profile_image_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_contact_profile_image_req_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}
	int idx = 0;
	idx++;	/* skip first value */
	gsize list_count = g_variant_n_children(res_info_struct[idx]);

	if (0 < list_count) {
		*images = (service_adaptor_profile_image_h *) calloc(list_count, sizeof(service_adaptor_profile_image_h));
		if (NULL == (*images)) {
			sac_error("Critical : Memory allocation failed!!");
		} else {
			for (int i = 0; i < list_count; i++) {
				(*images)[i] = (service_adaptor_profile_image_h)calloc(1, sizeof(struct _service_adaptor_profile_image_s));
				if (NULL == ((*images)[i])) {
					sac_error("Critical : Memory allocation failed!!");
				} else {
					GVariant *path_info_struct[3] = {NULL, };
					GVariant *path_info_entry_v = g_variant_get_child_value(res_info_struct[idx], i);
					path_info_struct[0] = g_variant_get_child_value(path_info_entry_v, 0);
					path_info_struct[1] = g_variant_get_child_value(path_info_entry_v, 1);
					path_info_struct[2] = g_variant_get_child_value(path_info_entry_v, 2);

					((*images)[i])->type	= g_variant_get_int32(path_info_struct[0]);
					((*images)[i])->index	= g_variant_get_int32(path_info_struct[1]);
					char *path		= ipc_g_variant_dup_string(path_info_struct[2]);;
					snprintf(((*images)[i])->path, 2048, "%s", (path ? path : ""));
					free(path);

					g_variant_unref(path_info_struct[0]);
					g_variant_unref(path_info_struct[1]);
					g_variant_unref(path_info_struct[2]);
				}
			}
		}
	}
	idx++;

	*images_len = g_variant_get_uint32(res_info_struct[idx++]);

	for (size_t j = 0; j < private_service_adaptor_contact_profile_image_req_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

GVariant *__create_privacy_req_type(const char *service_name,
						service_adaptor_privacy_req_s *privacy_req,
						void *user_data)
{
	GVariantBuilder *cts_builder = g_variant_builder_new(G_VARIANT_TYPE("a(ss)"));

	for (gsize j = 0; j < privacy_req->cts_len; j++) {
		g_variant_builder_open(cts_builder, G_VARIANT_TYPE("(ss)"));
		__safe_g_variant_builder_add_string(cts_builder, privacy_req->cts[j]->cc);
		__safe_g_variant_builder_add_string(cts_builder, privacy_req->cts[j]->pn);
		g_variant_builder_close(cts_builder);
	}

	GVariant *request = g_variant_new("(" private_service_adaptor_privacy_req_s_type ")", __safe_add_string(service_name), privacy_req->lvl, cts_builder, privacy_req->cts_len);

	g_variant_builder_unref(cts_builder);

	return request;
}

void __get_privacy_res_type(GVariant *call_result_struct,
						service_adaptor_privacy_res_s **privacy_res,
						void **server_data)
{
	GVariant *res_info_struct[private_service_adaptor_privacy_res_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_privacy_res_s_type_length; j++) {
		res_info_struct[j] = g_variant_get_child_value(call_result_struct, j);
	}

	int idx = 0;
	*privacy_res = (service_adaptor_privacy_res_s *) calloc(1, sizeof(service_adaptor_privacy_res_s));
	if (NULL != *privacy_res) {
		(*privacy_res)->lvl = g_variant_get_uint32(res_info_struct[idx++]);
		(*privacy_res)->prscon = g_variant_get_uint32(res_info_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_privacy_res_s_type_length; j++) {
		g_variant_unref(res_info_struct[j]);
	}
}

void __get_profile_type_res_type(GVariant *call_result_struct,
						char **url)
{
	GVariant *res_info_struct = NULL;

	res_info_struct = g_variant_get_child_value(call_result_struct, 0);

	*url = ipc_g_variant_dup_string(res_info_struct);

	g_variant_unref(res_info_struct);
}


/******************************************************************************
 * Public interface definition
 ******************************************************************************/

/**	@brief
 *	@return	service_adaptor_error_s
 *	@remarks :
 */
int _dbus_set_new_contact_list(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_contact_req_type(service_name, contact_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_NEW_CONTACT_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_contact_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != contact_res) {
					__get_contact_res_type(call_result_struct[0], contact_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_contact_list(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_contact_req_type(service_name, contact_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_CONTACT_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_contact_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != contact_res) {
					__get_contact_res_type(call_result_struct[0], contact_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_contact_list(const char *service_name,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_essential_s_type ")", __safe_add_string(service_name));

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_CONTACT_LIST_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_contact_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != contact_res) {
					__get_contact_res_type(call_result_struct[0], contact_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_contact_infos_polling(const char *service_name,
						service_adaptor_contact_req_s *contact_req,
						void *user_data,
						service_adaptor_contact_res_s **contact_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_contact_req_type(service_name, contact_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_CONTACT_INFOS_POLLING_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_contact_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != contact_res) {
					__get_contact_res_type(call_result_struct[0], contact_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_me_profile_with_push(const char *service_name,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_profile_req_type(service_name, profile_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_ME_PROFILE_WITH_PUSH_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_profile(const char *service_name,
						service_adaptor_profile_req_s *profile_req,
						void *user_data,
						service_adaptor_profile_res_s **profile_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_profile_req_type(service_name, profile_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_PROFILE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_profile_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != profile_res) {
					__get_profile_res_type(call_result_struct[0], profile_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_profile_image_meta_with_push(const char *service_name,
						service_adaptor_profile_image_h *images,
						unsigned int images_len,
						void *user_data,
						service_adaptor_profile_image_h **failed_images,
						unsigned int *failed_images_len,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_file_path_req_type(service_name, images, images_len, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_PROFILE_IMAGE_META_WITH_PUSH_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_contact_profile_image_req_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
				if ((NULL != failed_images) && (NULL != failed_images_len)) {
					__get_failed_image_meta_res_type(call_result_struct[0], failed_images, failed_images_len);
				}
			} else {
				/* __get_failed_image_meta_res_type(call_result_struct[0], failed_images, failed_images_len); */
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_delete_me_profile_image_meta_with_push(const char *service_name,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_del_me_profile_req_type(service_name, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_DEL_ME_PROFILE_IMAGE_META_WITH_PUSH_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_me_profile_privacy(const char *service_name,
						service_adaptor_privacy_req_s *privacy_req,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = __create_privacy_req_type(service_name, privacy_req, user_data);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_ME_PROFILE_PRIVACY_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_get_me_profile_privacy(const char *service_name,
						service_adaptor_privacy_res_s **privacy_res,
						void **server_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_essential_s_type ")", __safe_add_string(service_name));

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_GET_PROFILE_PRIVACY_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_privacy_res_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				if (NULL != privacy_res) {
					__get_privacy_res_type(call_result_struct[0], privacy_res, server_data);
				}
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_me_presence_with_push(const char *service_name,
						service_adaptor_presence_req_s *presence_req,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_presence_req_s_type ")",
		__safe_add_string(service_name), __safe_add_string(presence_req->prsc),
		__safe_add_string(presence_req->status), presence_req->prscon);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_ME_PRESENCE_WITH_PUSH_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

int _dbus_set_me_presence_on_off_with_push(const char *service_name,
						service_adaptor_presence_req_s *presence_req,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_presence_req_s_type ")", __safe_add_string(service_name), __safe_add_string(presence_req->prsc), __safe_add_string(presence_req->status), presence_req->prscon);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_ME_PRESENCE_ON_OFF_WITH_PUSH_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE("(ts)"))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[2];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[0]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[1]);
				ret = _get_result_code(remote_call_result);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}


int _dbus_set_me_profile_type(const char *service_name,
						int type,
						char **url,
						void *user_data,
						service_adaptor_error_s *error)
{
	int ret = SERVICE_ADAPTOR_ERROR_NONE;
	GError *g_error = NULL;
	GVariant *call_result = NULL;

	GDBusProxy *sac_interface_proxy = _dbus_get_sac_interface_proxy();

	ipc_check_proxy(sac_interface_proxy);

	GVariant *request = g_variant_new("(" private_service_adaptor_set_me_profile_type_req_s_type ")", __safe_add_string(service_name), type);

	call_result = g_dbus_proxy_call_sync(sac_interface_proxy,
			PRIVATE_DBUS_SET_ME_PROFILE_TYPE_METHOD,
			request,
			G_DBUS_CALL_FLAGS_NONE,
			G_MAXINT,
			NULL,
			&g_error);

	if (NULL == call_result) {
		error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;

		if (NULL != g_error) {
			error->msg = __SAFE_STRDUP(g_error->message);
			g_error_free(g_error);
		}
	} else {
		if (FALSE == g_variant_is_of_type(call_result, G_VARIANT_TYPE(MAKE_RETURN_TYPE(private_service_adaptor_essential_s_type)))) {
			error->code = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
			error->msg = strdup("D-Bus return type error");
			ret = SERVICE_ADAPTOR_ERROR_IPC_UNSTABLE;
		} else {
			GVariant *call_result_struct[3];
			call_result_struct[0] = g_variant_get_child_value(call_result, 0);
			call_result_struct[1] = g_variant_get_child_value(call_result, 1);
			call_result_struct[2] = g_variant_get_child_value(call_result, 2);

			uint64_t remote_call_result = g_variant_get_uint64(call_result_struct[1]);

			if (SERVICE_ADAPTOR_ERROR_NONE != remote_call_result) {
				error->code = remote_call_result;
				error->msg = ipc_g_variant_dup_string(call_result_struct[2]);
				ret = _get_result_code(remote_call_result);
			} else {
				__get_profile_type_res_type(call_result_struct[0], url);
			}

			g_variant_unref(call_result_struct[0]);
			g_variant_unref(call_result_struct[1]);
			g_variant_unref(call_result_struct[2]);
		}

		g_variant_unref(call_result);
	}

	return ret;
}

