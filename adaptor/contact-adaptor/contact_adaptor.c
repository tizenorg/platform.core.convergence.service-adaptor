#include <stdio.h>
#include <glib.h>

#include "contact_adaptor.h"
#include "service_adaptor_internal.h"

contact_adaptor_h contact_adaptor_create()
{
	contact_adaptor_h contact = (contact_adaptor_h) g_malloc0(sizeof(contact_adaptor_s));

	return contact;
}

service_adaptor_error_e contact_adaptor_destroy(contact_adaptor_h contact)
{
	SAL_FREE(contact);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e contact_adaptor_start(contact_adaptor_h contact)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e contact_adaptor_stop(contact_adaptor_h contact)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e contact_adaptor_register_listener(contact_adaptor_h contact, contact_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e contact_adaptor_unregister_listener(contact_adaptor_h contact, contact_adaptor_listener_h listener)
{
	return SERVICE_ADAPTOR_ERROR_NONE;
}
