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

#ifndef __SERVICE_ADAPTOR_PLUGIN_H__
#define __SERVICE_ADAPTOR_PLUGIN_H__

#include <tizen.h>
#include "service-adaptor-type.h"

/* TODO It will be re-define another header */
#define SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_VERSION	"http://tizen.org/service-adaptor/version"
#define SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_AUTH	"http://tizen.org/service-adaptor/auth"
#define SERVICE_ADAPTOR_3RD_PARTY_METADATA_KEY_STORAGE	"http://tizen.org/service-adaptor/storage"


service_adaptor_internal_error_code_e service_adaptor_set_package_installed_callback(service_adaptor_h _service_adaptor);

service_adaptor_internal_error_code_e service_adaptor_scan_all_packages(service_adaptor_h _service_adaptor);

service_adaptor_internal_error_code_e service_adaptor_scan_all_packages_async(service_adaptor_h _service_adaptor);

#endif /* __SERVICE_ADAPTOR_PLUGIN_H__ */
