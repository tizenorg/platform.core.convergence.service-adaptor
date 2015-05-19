#include <stdio.h>
#include <glib.h>

#include "storage_adaptor.h"
#include "service_adaptor_internal.h"

storage_adaptor_h storage_adaptor_create()
{
	storage_adaptor_h storage = (storage_adaptor_h) g_malloc0(sizeof(storage_adaptor_s));

	return storage;
}

service_adaptor_error_e storage_adaptor_destroy(storage_adaptor_h storage)
{
	SAL_FREE(storage);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e storage_adaptor_start(storage_adaptor_h storage)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e storage_adaptor_stop(storage_adaptor_h storage)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e storage_adaptor_register_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e storage_adaptor_unregister_listener(storage_adaptor_h storage, storage_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}
