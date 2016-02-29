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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <sys/time.h>

#include "service-adaptor.h"
#include "service-adaptor-contact.h"
#include "service-adaptor-storage.h"
#include "service-adaptor-type.h"
#include "service-adaptor-log.h"
#include "dbus-contact-adaptor.h"
#include "dbus-server.h"
#include "dbus-util.h"

char *ipc_g_variant_dup_contact_string(GVariant *string)
{
        char *ret = g_variant_dup_string(string, NULL);
	const int contact_protocol_padding_length = 1;
        if (0 == strcmp(ret, "")) {
                free(ret);
                ret = NULL;
        } else if (0 < strlen(ret)) {
		char *dummy = ret;
		ret = strdup((dummy + contact_protocol_padding_length));
		free(dummy);
	}

        return ret;
}


static void __separate_path_to_dir_base(char *full_path,
						char **dir_path,
						char **base_path)
{
	if ((NULL == full_path) || (0 >= strlen(full_path))) {
		*base_path = strdup("");
		*dir_path = strdup("");
		return;
	}

	char *base = strrchr(full_path, '/');
	if (NULL == base) {
		*base_path = strdup(full_path);
		*dir_path = strdup("");
	} else if (0 == strcmp(full_path, base)) {
		*base_path = strdup(full_path);
		*dir_path = strdup("");
	} else {
		*base_path = strdup(base + 1);
		if ((base - full_path) > 1) {
			*dir_path = strndup(full_path, (base - full_path));
		} else {
			*dir_path = strdup("");
		}
	}
}

void __destroy_req_images(contact_adaptor_contact_image_h *imgs, unsigned int imgs_len)
{
	if (NULL == imgs) {
		return;
	}

	for (int i = 0; i < imgs_len; i++) {
		if (NULL != imgs[i]) {
			free(imgs[i]->img);
			free(imgs[i]);
		}
	}
	free(imgs);
}

void __get_contact_req_type(GVariant *parameters,
						char **service_name,
						contact_adaptor_contact_req_h *contact_req)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_contact_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_contact_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	*contact_req = (contact_adaptor_contact_req_h) calloc(1, sizeof(contact_adaptor_contact_req_t));
	if (NULL != (*contact_req)) {
		(*contact_req)->tt = g_variant_get_int64(req_struct[idx++]);

		gsize list_count = g_variant_n_children(req_struct[idx]);

FUNC_STEP();
		(*contact_req)->cts = (contact_adaptor_contact_info_req_h *) calloc(list_count, sizeof(contact_adaptor_contact_info_req_h));

		if (NULL != ((*contact_req)->cts)) {
			for (gsize i = 0; i < list_count; i++) {
				GVariant *req_info_struct[private_service_adaptor_contact_info_req_s_type_length];
				GVariant *req_info_entry_v = g_variant_get_child_value(req_struct[idx], i);

				contact_adaptor_contact_info_req_h contact_info_req
						= (contact_adaptor_contact_info_req_h) calloc(1, sizeof(contact_adaptor_contact_info_req_t));

				if (NULL != contact_info_req) {
					for (size_t j = 0; j < private_service_adaptor_contact_info_req_s_type_length; j++) {
						req_info_struct[j] = g_variant_get_child_value(req_info_entry_v, j);
					}

					int idx2 = 0;
					contact_info_req->tp = ipc_g_variant_dup_string(req_info_struct[idx2++]);
					contact_info_req->id = ipc_g_variant_dup_string(req_info_struct[idx2++]);
					contact_info_req->pn = ipc_g_variant_dup_string(req_info_struct[idx2++]);
					contact_info_req->nm = ipc_g_variant_dup_contact_string(req_info_struct[idx2++]);
					contact_info_req->cc = ipc_g_variant_dup_string(req_info_struct[idx2++]);

					for (size_t j = 0; j < private_service_adaptor_contact_info_req_s_type_length; j++) {
						g_variant_unref(req_info_struct[j]);
					}
				}
				(*contact_req)->cts[i] = contact_info_req;
			}
		}
		idx++;

FUNC_STEP();
		(*contact_req)->cts_len = g_variant_get_uint32(req_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_contact_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

GVariant *__create_contact_res_type(contact_adaptor_contact_res_h contact_res,
						contact_adaptor_error_code_h error_code)
{
FUNC_START();
	contact_adaptor_contact_res_t _contact_res;
	_contact_res.tt = 0;
	_contact_res.cts_len = 0;

	if (NULL == contact_res) {
		contact_res = &_contact_res;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE(private_contact_info_res_list_type));
	GVariantBuilder *builder_in;
FUNC_STEP();

	for (gsize i = 0; i < contact_res->cts_len; i++) {
		g_variant_builder_open(builder, G_VARIANT_TYPE(private_service_adaptor_contact_info_res_s_type));
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->duid);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->id);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->msisdn);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->ty);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->cc);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->pn);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->nm);

		/* evnt */
FUNC_STEP();
		builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

		if (NULL != contact_res->cts[i]->evnt) {
			service_adaptor_debug("event_len  : %d", contact_res->cts[i]->evnt_len);
			for (gsize j = 0; j < contact_res->cts[i]->evnt_len; j++) {
				safe_g_variant_builder_add_array_string(builder_in, contact_res->cts[i]->evnt[j]);
			}
		} else {
			service_adaptor_debug("(event == NULL) => event_len = 0");
			contact_res->cts[i]->evnt_len = 0U;
		}

		g_variant_builder_add(builder, "a(s)", builder_in);
		g_variant_builder_unref(builder_in);

		g_variant_builder_add(builder, "u", contact_res->cts[i]->evnt_len);

		/* imgs */
		builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(is)"));
		if (NULL != contact_res->cts[i]->imgs) {
			service_adaptor_debug("image_len  : %d", contact_res->cts[i]->imgs_len);
			for (gsize j = 0; j < contact_res->cts[i]->imgs_len; j++) {
				g_variant_builder_add(builder_in, "(is)", contact_res->cts[i]->imgs[j]->no, __safe_add_string(contact_res->cts[i]->imgs[j]->img));
			}
		} else {
			service_adaptor_debug("(image == NULL) => image_len = 0");
			contact_res->cts[i]->imgs_len = 0U;
		}
		g_variant_builder_add(builder, "a(is)", builder_in);
		g_variant_builder_unref(builder_in);

		g_variant_builder_add(builder, "u", contact_res->cts[i]->imgs_len);

		/* adrs */
FUNC_STEP();
		builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

		if (NULL != contact_res->cts[i]->adrs) {
			service_adaptor_debug("adrs_len  : %d", contact_res->cts[i]->adrs_len);
			for (gsize j = 0; j < contact_res->cts[i]->adrs_len; j++) {
				safe_g_variant_builder_add_array_string(builder_in, contact_res->cts[i]->adrs[j]);
			}
		} else {
			service_adaptor_debug("(adrs == NULL) => adrs_len = 0");
			contact_res->cts[i]->adrs_len = 0U;
		}

		g_variant_builder_add(builder, "a(s)", builder_in);
		g_variant_builder_unref(builder_in);

		g_variant_builder_add(builder, "u", contact_res->cts[i]->adrs_len);

		/* mail */
FUNC_STEP();
		builder_in = g_variant_builder_new(G_VARIANT_TYPE("a(s)"));

		if (NULL != contact_res->cts[i]->mail) {
			service_adaptor_debug("mail_len  : %d", contact_res->cts[i]->mail_len);
			for (gsize j = 0; j < contact_res->cts[i]->mail_len; j++) {
				safe_g_variant_builder_add_array_string(builder_in, contact_res->cts[i]->mail[j]);
			}
		} else {
			service_adaptor_debug("(mail == NULL) => mail_len = 0");
			contact_res->cts[i]->mail_len = 0U;
		}

		g_variant_builder_add(builder, "a(s)", builder_in);
		g_variant_builder_unref(builder_in);

		g_variant_builder_add(builder, "u", contact_res->cts[i]->mail_len);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->org);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->prsc);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->status);
		g_variant_builder_add(builder, "u", contact_res->cts[i]->sids);
FUNC_STEP();
		g_variant_builder_add(builder, "i", contact_res->cts[i]->type);
		safe_g_variant_builder_add_string(builder, contact_res->cts[i]->url);

		g_variant_builder_close(builder);
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_contact_res_s_type), (int64_t)contact_res->tt, builder, (uint32_t)contact_res->cts_len, (uint64_t)error_code->code, __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

FUNC_END();
	return response;
}

void __get_profile_req_type(GVariant *parameters,
						char **service_name,
						contact_adaptor_profile_req_h *profile_req)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_profile_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_profile_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

FUNC_STEP();
	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	*profile_req = (contact_adaptor_profile_req_h) calloc(1, sizeof(contact_adaptor_profile_req_t));
	if (NULL != (*profile_req)) {
		(*profile_req)->cc = ipc_g_variant_dup_string(req_struct[idx++]);
		(*profile_req)->pn = ipc_g_variant_dup_string(req_struct[idx++]);
		(*profile_req)->nm = ipc_g_variant_dup_contact_string(req_struct[idx++]);

		gsize evnt_count = g_variant_n_children(req_struct[idx]);

		FUNC_STEP();
		(*profile_req)->evnt = (char **) calloc(evnt_count, sizeof(char *));

		if (NULL != ((*profile_req)->evnt)) {
			for (gsize i = 0; i < evnt_count; i++) {
				GVariant *evnt_info_struct;
				GVariant *evnt_info_entry_v = g_variant_get_child_value(req_struct[idx], i);
				evnt_info_struct = g_variant_get_child_value(evnt_info_entry_v, 0);

				(*profile_req)->evnt[i] = ipc_g_variant_dup_string(evnt_info_struct);

				g_variant_unref(evnt_info_struct);
			}
		}
		idx++;

		(*profile_req)->evnt_len = g_variant_get_uint32(req_struct[idx++]);
		(*profile_req)->img = ipc_g_variant_dup_string(req_struct[idx++]);

		FUNC_STEP();
		gsize adrs_count = g_variant_n_children(req_struct[idx]);

		(*profile_req)->adrs = (char **) calloc(adrs_count, sizeof(char *));

		if (NULL != ((*profile_req)->adrs)) {
			for (gsize i = 0; i < adrs_count; i++) {
				GVariant *adrs_info_struct;
				GVariant *adrs_info_entry_v = g_variant_get_child_value(req_struct[idx], i);
				adrs_info_struct = g_variant_get_child_value(adrs_info_entry_v, 0);

				(*profile_req)->adrs[i] = ipc_g_variant_dup_string(adrs_info_struct);

				g_variant_unref(adrs_info_struct);
			}
		}
		idx++;

		FUNC_STEP();
		(*profile_req)->adrs_len = g_variant_get_uint32(req_struct[idx++]);

		gsize mail_count = g_variant_n_children(req_struct[idx]);

		(*profile_req)->mail = (char **) calloc(mail_count, sizeof(char *));

		if (NULL != ((*profile_req)->mail)) {
			for (gsize i = 0; i < mail_count; i++) {
				GVariant *mail_info_struct;
				GVariant *mail_info_entry_v = g_variant_get_child_value(req_struct[idx], i);
				mail_info_struct = g_variant_get_child_value(mail_info_entry_v, 0);

				(*profile_req)->mail[i] = ipc_g_variant_dup_string(mail_info_struct);

				g_variant_unref(mail_info_struct);
			}
		}
		idx++;

		(*profile_req)->mail_len = g_variant_get_uint32(req_struct[idx++]);
		(*profile_req)->org = ipc_g_variant_dup_string(req_struct[idx++]);
		(*profile_req)->prsc = ipc_g_variant_dup_string(req_struct[idx++]);
		(*profile_req)->status = ipc_g_variant_dup_string(req_struct[idx++]);
	}
	for (size_t j = 0; j < private_service_adaptor_profile_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

GVariant *__create_profile_res_type(contact_adaptor_profile_res_h profile_res,
						contact_adaptor_error_code_h error_code)
{
FUNC_START();
	contact_adaptor_profile_res_t _profile_res;
	_profile_res.nm = "";
	_profile_res.img = "";
	_profile_res.prsc = "";

	if (NULL == profile_res) {
		profile_res = &_profile_res;
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_profile_res_s_type), __safe_add_string(profile_res->nm), __safe_add_string(profile_res->img), __safe_add_string(profile_res->prsc), __safe_add_string(profile_res->status), (uint64_t) error_code->code, __safe_add_string(error_code->msg));

	/* free error and res */

FUNC_END();
	return response;
}

void __get_delete_me_req_type(GVariant *parameters,
						char **service_name)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_essential_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
FUNC_STEP();

	for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

void __get_set_me_profile_type_req_type(GVariant *parameters,
						char **service_name,
						int *type)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_set_me_profile_type_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_set_me_profile_type_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
	*type = g_variant_get_int32(req_struct[idx++]);
FUNC_STEP();

	for (size_t j = 0; j < private_service_adaptor_set_me_profile_type_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}


void __get_file_path_req_type(GVariant *parameters,
						char **service_name,
						contact_adaptor_contact_image_h **req_imgs,
						unsigned int *req_imgs_len,
						contact_adaptor_contact_image_h **res_imgs,
						unsigned int *res_imgs_len)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_contact_profile_image_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_contact_profile_image_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);
FUNC_STEP();

	gsize list_count = g_variant_n_children(req_struct[idx]);

	*req_imgs = (contact_adaptor_contact_image_h *) calloc(list_count, sizeof(contact_adaptor_contact_image_h));
	*res_imgs = (contact_adaptor_contact_image_h *) calloc(list_count, sizeof(contact_adaptor_contact_image_h));
	if ((NULL != (*req_imgs)) && (NULL != (*res_imgs))) {
		for (gsize i = 0; i < list_count; i++) {
			(*req_imgs)[i] = (contact_adaptor_contact_image_h) calloc(1, sizeof(contact_adaptor_contact_image_t));
			(*res_imgs)[i] = NULL;
			if (NULL != (*req_imgs)[i]) {
				GVariant *path_info_struct;
				GVariant *path_info_entry_v = g_variant_get_child_value(req_struct[idx], i);
				path_info_struct = g_variant_get_child_value(path_info_entry_v, 0);
				(*req_imgs)[i]->req_type = g_variant_get_int32(path_info_struct);

				path_info_struct = g_variant_get_child_value(path_info_entry_v, 1);
				(*req_imgs)[i]->no = g_variant_get_int32(path_info_struct);

				path_info_struct = g_variant_get_child_value(path_info_entry_v, 2);
				(*req_imgs)[i]->img = ipc_g_variant_dup_string(path_info_struct);

				g_variant_unref(path_info_struct);
			}
		}
		idx++;
		*req_imgs_len = g_variant_get_uint32(req_struct[idx++]);
	} else {
		free(*req_imgs);
		free(*res_imgs);
		*req_imgs = NULL;
		*res_imgs = NULL;
		*req_imgs_len = 0U;
		*res_imgs_len = 0U;
	}

	for (size_t j = 0; j < private_service_adaptor_contact_profile_image_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

GVariant *__create_file_path_res_type(contact_adaptor_contact_image_h *res_imgs,
						unsigned int res_imgs_len,
						contact_adaptor_error_code_h error_code)
{
FUNC_START();
	if (NULL == res_imgs) {
		res_imgs_len = 0;
	}

	GVariantBuilder *builder = g_variant_builder_new(G_VARIANT_TYPE("a(iis)"));

	for (gsize j = 0; j < res_imgs_len; j++) {
		if (NULL != res_imgs[j]) {
			g_variant_builder_add(builder, "(iis)", res_imgs[j]->req_type, res_imgs[j]->no, __safe_add_string(res_imgs[j]->img));
		}
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_contact_profile_image_req_s_type), "", builder, (uint32_t)res_imgs_len, (uint64_t) error_code->code, __safe_add_string(error_code->msg));

	g_variant_builder_unref(builder);

FUNC_END();
	return response;
}

void __get_privacy_req_type(GVariant *parameters,
						char **service_name,
						contact_adaptor_privacy_req_h *privacy_req)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_privacy_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_privacy_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	*privacy_req = (contact_adaptor_privacy_req_h) calloc(1, sizeof(contact_adaptor_privacy_req_t));
	if (NULL != (*privacy_req)) {
		(*privacy_req)->lvl = g_variant_get_uint32(req_struct[idx++]);

		gsize list_count = g_variant_n_children(req_struct[idx]);

		(*privacy_req)->cts = (contact_adaptor_privacy_info_req_h *) calloc(list_count, sizeof(contact_adaptor_privacy_info_req_h));

		if (NULL != ((*privacy_req)->cts)) {
			for (gsize i = 0; i < list_count; i++) {
				FUNC_STEP();
				GVariant *req_info_struct[private_service_adaptor_privacy_info_req_s_type_length];
				GVariant *req_info_entry_v = g_variant_get_child_value(req_struct[idx], i);

				for (size_t j = 0; j < private_service_adaptor_privacy_info_req_s_type_length; j++) {
					req_info_struct[j] = g_variant_get_child_value(req_info_entry_v, j);
				}

				int idx2 = 0;
				(*privacy_req)->cts[i] = (contact_adaptor_privacy_info_req_h) calloc(1, sizeof(contact_adaptor_privacy_info_req_t));
				if (NULL != ((*privacy_req)->cts[i])) {
					(*privacy_req)->cts[i]->cc = ipc_g_variant_dup_string(req_info_struct[idx2++]);
					(*privacy_req)->cts[i]->pn = ipc_g_variant_dup_string(req_info_struct[idx2++]);
				}

				for (size_t j = 0; j < private_service_adaptor_privacy_info_req_s_type_length; j++) {
					g_variant_unref(req_info_struct[j]);
				}
			}
		}
		idx++;

		(*privacy_req)->cts_len = g_variant_get_uint32(req_struct[idx++]);
	}
	for (size_t j = 0; j < private_service_adaptor_privacy_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

GVariant *__create_privacy_res_type(contact_adaptor_privacy_res_h privacy_res,
						contact_adaptor_error_code_h error_code)
{
FUNC_START();
	contact_adaptor_privacy_res_t _privacy_res;
	_privacy_res.lvl = 0;
	_privacy_res.prscon = 0;

	if (NULL == privacy_res) {
		privacy_res = &_privacy_res;
	}

	GVariant *response = g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_privacy_res_s_type), privacy_res->lvl, privacy_res->prscon, (uint64_t) error_code->code, __safe_add_string(error_code->msg));

FUNC_END();
	return response;
}

void __get_presence_req_type(GVariant *parameters,
						char **service_name,
						contact_adaptor_presence_info_h *presence_req)
{
FUNC_START();
	GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
	GVariant *req_struct[private_service_adaptor_presence_req_s_type_length];

	for (size_t j = 0; j < private_service_adaptor_presence_req_s_type_length; j++) {
		req_struct[j] = g_variant_get_child_value(in_parameters, j);
	}

	int idx = 0;
	*service_name = ipc_g_variant_dup_string(req_struct[idx++]);

	*presence_req = (contact_adaptor_presence_info_h) calloc(1, sizeof(contact_adaptor_presence_info_t));
	if (NULL != (*presence_req)) {
		(*presence_req)->prsc = ipc_g_variant_dup_string(req_struct[idx++]);
		(*presence_req)->status = ipc_g_variant_dup_string(req_struct[idx++]);
		(*presence_req)->prscon = g_variant_get_uint32(req_struct[idx++]);
	}

	for (size_t j = 0; j < private_service_adaptor_presence_req_s_type_length; j++) {
		g_variant_unref(req_struct[j]);
	}
FUNC_END();
}

void contact_adaptor_method_call(GDBusConnection *connection,
						const gchar *sender,
						const gchar *object_path,
						const gchar *interface_name,
						const gchar *method_name,
						GVariant *parameters,
						GDBusMethodInvocation *invocation,
						gpointer user_data)
{
FUNC_START();
	service_adaptor_internal_error_code_e ret_code = SERVICE_ADAPTOR_INTERNAL_ERROR_NONE;

	if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_NEW_CONTACT_LIST_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_contact_req_h contact_req = NULL;
		contact_adaptor_contact_res_h contact_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_contact_req_type(parameters, &service_name, &contact_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_contact_res_type(contact_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_contact_req_s(contact_req);
			contact_adaptor_destroy_contact_res_s(contact_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_new_contact_list(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_new_contact_list(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);
			}

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NONE != ret_code) {
				service_adaptor_error("Can not run contact_adaptor_new_contact_list()");
			}
		}
FUNC_STEP();

		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_contact_res_type(contact_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_contact_req_s(contact_req);
		contact_adaptor_destroy_contact_res_s(contact_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_CONTACT_LIST_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_contact_req_h contact_req = NULL;
		contact_adaptor_contact_res_h contact_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_contact_req_type(parameters, &service_name, &contact_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_contact_res_type(contact_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_contact_req_s(contact_req);
			contact_adaptor_destroy_contact_res_s(contact_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_contact_list(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_contact_list(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_contact_res_type(contact_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_contact_req_s(contact_req);
		contact_adaptor_destroy_contact_res_s(contact_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_CONTACT_LIST_METHOD)) {
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_essential_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;
		char *service_name = ipc_g_variant_dup_string(req_struct[idx++]);

		for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
			g_variant_unref(req_struct[j]);
		}

		service_adaptor_debug("(%s)", service_name);

		contact_adaptor_contact_res_h contact_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_contact_res_type(contact_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_contact_res_s(contact_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_get_contact_infos_latest(plugin, service->contact_context, NULL, NULL, &contact_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_get_contact_infos_latest(plugin, service->contact_context, NULL, NULL, &contact_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_contact_res_type(contact_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_contact_res_s(contact_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_CONTACT_INFOS_POLLING_METHOD)) {
		char *service_name = NULL;

		contact_adaptor_contact_req_h contact_req = NULL;
		contact_adaptor_contact_res_h contact_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_contact_req_type(parameters, &service_name, &contact_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_contact_res_type(contact_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_contact_req_s(contact_req);
			contact_adaptor_destroy_contact_res_s(contact_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_get_contact_infos_polling(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_get_contact_infos_polling(plugin, service->contact_context, contact_req, NULL, &contact_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_contact_res_type(contact_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_contact_req_s(contact_req);
		contact_adaptor_destroy_contact_res_s(contact_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_ME_PROFILE_WITH_PUSH_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_profile_req_h profile_req = NULL;
		contact_adaptor_profile_res_h profile_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_profile_req_type(parameters, &service_name, &profile_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			contact_adaptor_destroy_profile_req_s(profile_req);
			contact_adaptor_destroy_profile_res_s(profile_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_me_profile_with_push(plugin, service->contact_context, profile_req, NULL, &profile_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_me_profile_with_push(plugin, service->contact_context, profile_req, NULL, &profile_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		service_adaptor_debug("(%s)", service_name);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_profile_req_s(profile_req);
		contact_adaptor_destroy_profile_res_s(profile_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_PROFILE_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_profile_req_h profile_req = NULL;
		contact_adaptor_profile_res_h profile_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_profile_req_type(parameters, &service_name, &profile_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_profile_res_type(profile_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_profile_req_s(profile_req);
			contact_adaptor_destroy_profile_res_s(profile_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_get_profile(plugin, service->contact_context, profile_req, NULL, &profile_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_get_profile(plugin, service->contact_context, profile_req, NULL, &profile_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_profile_res_type(profile_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_profile_req_s(profile_req);
		contact_adaptor_destroy_profile_res_s(profile_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_PROFILE_IMAGE_META_WITH_PUSH_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_contact_image_h *req_imgs = NULL;
		unsigned int req_imgs_len = 0;
		contact_adaptor_contact_image_h *res_imgs = NULL;
		unsigned int res_imgs_len = 0;

		contact_adaptor_error_code_h contact_error_code = NULL;
		storage_adaptor_error_code_h storage_error_code = NULL;

		contact_adaptor_error_code_t _contact_error;
		_contact_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_contact_error.msg = NULL;

		__get_file_path_req_type(parameters, &service_name, &req_imgs, &req_imgs_len, &res_imgs, &res_imgs_len);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			contact_error_code = &_contact_error;
			contact_error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			contact_error_code->msg = "Can not get service context";

			GVariant *response = __create_file_path_res_type(res_imgs, res_imgs_len, contact_error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			__destroy_req_images(req_imgs, req_imgs_len);
			__destroy_req_images(res_imgs, res_imgs_len);
			free(service_name);
			return;
		}

		contact_adaptor_h contact_adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h contact_plugin = NULL;

		if (NULL != service->contact_context) {
			contact_plugin = contact_adaptor_get_plugin_by_name(contact_adaptor, service->contact_context->plugin_uri);
		}

		storage_adaptor_h storage_adaptor = service_adaptor_get_storage_adaptor(service_adaptor);
		storage_adaptor_plugin_h storage_plugin = NULL;

		if (NULL != service->storage_context) {
			storage_plugin = storage_adaptor_get_plugin_by_name(storage_adaptor, service->storage_context->plugin_uri);
		}

		if ((NULL != contact_adaptor) && (NULL != contact_plugin) && (NULL != storage_adaptor)
				&& (NULL != storage_plugin) && (NULL != req_imgs) && (0 < req_imgs_len)) {
			FUNC_STEP();
			for (int i = 0; i < req_imgs_len; i++) {
				if (CONTACT_ADAPTOR_REQUEST_SET != req_imgs[i]->req_type) {
					service_adaptor_debug("Skip upload to %dth image (delete operation)", i);
					continue;
				}
				char *parent_folder = NULL;
				char *full_path = req_imgs[i]->img;
				char *folder_path = NULL, *file_name = NULL;
				struct timeval tv;
				gettimeofday(&tv, NULL);

				__separate_path_to_dir_base(full_path, &folder_path, &file_name);
				parent_folder = g_strdup_printf("/contact%s/%ld%ld", folder_path, (long)tv.tv_sec, (long)tv.tv_usec);
				storage_adaptor_file_info_h file_info = NULL;
				service_adaptor_info("Contact upload path : prn(%s) file(%s)", parent_folder, file_name);

				storage_adaptor_destroy_error_code(&storage_error_code);
				ret_code = storage_adaptor_upload_file_sync(storage_plugin, service->storage_context,
						parent_folder, file_name, req_imgs[i]->img, true, NULL, &file_info, &storage_error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->storage_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%lld: %s)", storage_error_code ? storage_error_code->code : 0ULL,
							storage_error_code ? storage_error_code->msg : NULL);
					storage_adaptor_destroy_error_code(&storage_error_code);

					service_adaptor_debug("Re-try API");
					ret_code = storage_adaptor_upload_file_sync(storage_plugin, service->storage_context,
							parent_folder, file_name, req_imgs[i]->img, true, NULL, &file_info, &storage_error_code, NULL);
				}

				if ((NULL == storage_error_code) && (NULL != file_info) && (NULL != file_info->file_share_token)) {
					FUNC_STEP();
					GString* public_filepath = g_string_new("");
					g_string_append_printf(public_filepath, "%s?auth_code=%s",
							file_info->file_share_token->public_token, file_info->file_share_token->auth_code);
					free(req_imgs[i]->img);
					req_imgs[i]->img = public_filepath->str;

					storage_adaptor_destroy_file_info(&file_info);
					service_adaptor_info("Shared image url (%s)", req_imgs[i]->img);
				} else {
					if (NULL != storage_error_code) {
						service_adaptor_error("Upload error occured <%lld / %s>", storage_error_code->code, storage_error_code->msg);
					}
					res_imgs[res_imgs_len++] = req_imgs[i];
					req_imgs[i] = NULL;
				}
				storage_adaptor_destroy_file_info(&file_info);

				free(parent_folder);
				free(folder_path);
				free(file_name);
				FUNC_STEP();
			}

			contact_adaptor_contact_image_h *tmp_imgs = NULL;
			tmp_imgs = (contact_adaptor_contact_image_h *) calloc(req_imgs_len, sizeof(contact_adaptor_contact_image_h));
			unsigned int tmp_imgs_len = 0;

			if (NULL != tmp_imgs) {
				for (int al = 0; al < req_imgs_len; al++) {
					if (NULL != req_imgs[al]) {
						tmp_imgs[tmp_imgs_len++] = req_imgs[al];
					}
				}

				service_adaptor_info("%u imgs is requested", req_imgs_len);
				service_adaptor_info("%u imgs is failed", res_imgs_len);
				service_adaptor_info("%u imgs is success", tmp_imgs_len);

				ret_code = contact_adaptor_set_me_profile_image_meta_with_push(contact_plugin,
						service->contact_context, tmp_imgs, tmp_imgs_len, NULL, &contact_error_code, NULL);

				if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
					service_adaptor_info("API returns auth-error. Try refresh auth");
					service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

					service_adaptor_debug("Empty error_code already issued (%lld: %s)", contact_error_code ? contact_error_code->code : 0ULL,
							contact_error_code ? contact_error_code->msg : NULL);
					contact_adaptor_destroy_error_code(&contact_error_code);

					service_adaptor_debug("Re-try API");
					ret_code = contact_adaptor_set_me_profile_image_meta_with_push(contact_plugin,
							service->contact_context, tmp_imgs, tmp_imgs_len, NULL, &contact_error_code, NULL);
				}

			}
			free(tmp_imgs);
		}

		FUNC_STEP();
		if (NULL != storage_error_code) {
			contact_adaptor_destroy_error_code(&contact_error_code);

			contact_error_code = &_contact_error;
			contact_error_code->code = storage_error_code->code;
			contact_error_code->msg = storage_error_code->msg;
			free(storage_error_code);
		} else if (NULL == contact_error_code) {
			contact_error_code = &_contact_error;
			contact_error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			contact_error_code->msg = strdup("");
		}

		GVariant *response = __create_file_path_res_type(res_imgs, res_imgs_len, contact_error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (contact_error_code != &_contact_error) {
			free(contact_error_code->msg);
			free(contact_error_code);
			contact_error_code = NULL;
		} else {
			free(_contact_error.msg);
		}

		__destroy_req_images(req_imgs, req_imgs_len);
		__destroy_req_images(res_imgs, res_imgs_len);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_DEL_ME_PROFILE_IMAGE_META_WITH_PUSH_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_delete_me_req_type(parameters, &service_name);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_delete_me_profile_image_meta_with_push(plugin, service->contact_context, NULL, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_delete_me_profile_image_meta_with_push(plugin, service->contact_context,  NULL, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_ME_PROFILE_PRIVACY_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_privacy_req_h privacy_req = NULL;
		contact_adaptor_privacy_res_h privacy_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_privacy_req_type(parameters, &service_name, &privacy_req);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			contact_adaptor_destroy_privacy_req_s(privacy_req);
			contact_adaptor_destroy_privacy_res_s(privacy_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_me_profile_privacy(plugin, service->contact_context, privacy_req, NULL, &privacy_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_me_profile_privacy(plugin, service->contact_context, privacy_req, NULL, &privacy_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_privacy_req_s(privacy_req);
		contact_adaptor_destroy_privacy_res_s(privacy_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_GET_PROFILE_PRIVACY_METHOD)) {
		GVariant *in_parameters = g_variant_get_child_value(parameters, 0);
		GVariant *req_struct[private_service_adaptor_essential_s_type_length];

		for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
			req_struct[j] = g_variant_get_child_value(in_parameters, j);
		}

		int idx = 0;
		char *service_name = ipc_g_variant_dup_string(req_struct[idx++]);

		for (size_t j = 0; j < private_service_adaptor_essential_s_type_length; j++) {
			g_variant_unref(req_struct[j]);
		}

		service_adaptor_debug("(%s)", service_name);

		contact_adaptor_privacy_res_h privacy_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

FUNC_STEP();
		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			GVariant *response = __create_privacy_res_type(privacy_res, error_code);
			g_dbus_method_invocation_return_value(invocation, response);

			contact_adaptor_destroy_privacy_res_s(privacy_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_get_me_profile_privacy(plugin, service->contact_context, NULL, NULL, &privacy_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_get_me_profile_privacy(plugin, service->contact_context, NULL, NULL, &privacy_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		GVariant *response = __create_privacy_res_type(privacy_res, error_code);
		g_dbus_method_invocation_return_value(invocation, response);

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_privacy_res_s(privacy_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_ME_PRESENCE_WITH_PUSH_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_presence_info_h presence_req = NULL;

		__get_presence_req_type(parameters, &service_name, &presence_req);

		service_adaptor_debug("(%s)", service_name);

		contact_adaptor_presence_info_h presence_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

FUNC_STEP();
		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			contact_adaptor_destroy_presence_info_s(presence_req);
			contact_adaptor_destroy_presence_info_s(presence_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_me_presence_with_push(plugin, service->contact_context, presence_req, NULL, &presence_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_me_presence_with_push(plugin, service->contact_context, presence_req, NULL, &presence_res, &error_code, NULL);
			}
		}


FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_presence_info_s(presence_req);
		contact_adaptor_destroy_presence_info_s(presence_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_ME_PRESENCE_ON_OFF_WITH_PUSH_METHOD)) {
		char *service_name = NULL;
		contact_adaptor_presence_info_h presence_req = NULL;

		__get_presence_req_type(parameters, &service_name, &presence_req);

		service_adaptor_debug("(%s)", service_name);

		contact_adaptor_presence_info_h presence_res = NULL;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

FUNC_STEP();
		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			contact_adaptor_destroy_presence_info_s(presence_req);
			contact_adaptor_destroy_presence_info_s(presence_res);
			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_me_presence_on_off_with_push(plugin, service->contact_context, presence_req, NULL, &presence_res, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_me_presence_on_off_with_push(plugin, service->contact_context, presence_req, NULL, &presence_res, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new("(ts)", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		contact_adaptor_destroy_presence_info_s(presence_req);
		contact_adaptor_destroy_presence_info_s(presence_res);
		free(service_name);
	} else if (0 == g_strcmp0(method_name, PRIVATE_DBUS_SET_ME_PROFILE_TYPE_METHOD)) {
		char *service_name = NULL;
		int type = -1;
		contact_adaptor_error_code_h error_code = NULL;
		contact_adaptor_error_code_t _error;
		_error.code = CONTACT_ADAPTOR_ERROR_NONE;
		_error.msg = NULL;

		__get_set_me_profile_type_req_type(parameters, &service_name, &type);

		service_adaptor_debug("(%s)", service_name);

		service_adaptor_h service_adaptor = service_adaptor_get_handle();
		service_adaptor_service_context_h service =
				service_adaptor_get_service_context(service_adaptor, service_name);

		if (NULL == service) {
			service_adaptor_error("Can not get service context: %s", service_name);
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NOT_FOUND;
			error_code->msg = "Can not get service context";

			g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_essential_s_type), "", (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

			free(service_name);
			return;
		}

		contact_adaptor_h adaptor = service_adaptor_get_contact_adaptor(service_adaptor);
		contact_adaptor_plugin_h plugin = NULL;

		if (NULL != service->contact_context) {
			plugin = contact_adaptor_get_plugin_by_name(adaptor, service->contact_context->plugin_uri);
		}

		char *url = NULL;
		if ((NULL != adaptor) && (NULL != plugin)) {
FUNC_STEP();
			ret_code = contact_adaptor_set_me_profile_type(plugin, service->contact_context, type, NULL, &url, &error_code, NULL);

			if (SERVICE_ADAPTOR_INTERNAL_ERROR_NOT_AUTHORIZED == ret_code) {
				service_adaptor_info("API returns auth-error. Try refresh auth");
				service_adaptor_auth_refresh(service_adaptor, service_name, service->contact_context->plugin_uri);

				service_adaptor_debug("Empty error_code already issued (%lld: %s)", error_code ? error_code->code : 0ULL, error_code ? error_code->msg : NULL);
				contact_adaptor_destroy_error_code(&error_code);

				service_adaptor_debug("Re-try API");
				ret_code = contact_adaptor_set_me_profile_type(plugin, service->contact_context, type, NULL, &url, &error_code, NULL);
			}
		}

FUNC_STEP();
		if (NULL == error_code) {
			error_code = &_error;
			error_code->code = CONTACT_ADAPTOR_ERROR_NONE;
			error_code->msg = strdup("");
		}

		g_dbus_method_invocation_return_value(invocation, g_variant_new(MAKE_RETURN_TYPE(private_service_adaptor_essential_s_type),
				__safe_add_string(url), (uint64_t) error_code->code, __safe_add_string(error_code->msg)));

		if (error_code != &_error) {
			free(error_code->msg);
			free(error_code);
			error_code = NULL;
		} else {
			free(_error.msg);
		}
		free(url);
		free(service_name);
	}
FUNC_END();
}
