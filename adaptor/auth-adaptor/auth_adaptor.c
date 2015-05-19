#include <stdio.h>
#include <glib.h>

#include "auth_adaptor.h"
#include "service_adaptor_internal.h"

auth_adaptor_h auth_adaptor_create()
{
	auth_adaptor_h auth = (auth_adaptor_h) g_malloc0(sizeof(auth_adaptor_s));

	return auth;
}

service_adaptor_error_e auth_adaptor_destroy(auth_adaptor_h auth)
{
	SAL_FREE(auth);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e auth_adaptor_start(auth_adaptor_h auth)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e auth_adaptor_stop(auth_adaptor_h auth)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e auth_adaptor_register_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e auth_adaptor_unregister_listener(auth_adaptor_h auth, auth_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}
