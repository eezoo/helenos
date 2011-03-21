/*
 * Copyright (c) 2011 Vojtech Horky
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup usbinfo
 * @{
 */
/**
 * @file
 * Representation of queried device.
 */
#include <usb/pipes.h>
#include <errno.h>
#include <str_error.h>
#include <usb/request.h>
#include "usbinfo.h"

usbinfo_device_t *prepare_device(devman_handle_t hc_handle,
    usb_address_t dev_addr)
{
	usbinfo_device_t *dev = malloc(sizeof(usbinfo_device_t));
	if (dev == NULL) {
		fprintf(stderr, NAME ": memory allocation failed.\n");
		return NULL;
	}

	int rc;

	rc = usb_device_connection_initialize(&dev->wire, hc_handle, dev_addr);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": failed to create connection to the device: %s.\n",
		    str_error(rc));
		goto leave;
	}

	rc = usb_pipe_initialize_default_control(&dev->ctrl_pipe,
	    &dev->wire);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": failed to create default control pipe: %s.\n",
		    str_error(rc));
		goto leave;
	}

	rc = usb_pipe_probe_default_control(&dev->ctrl_pipe);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": probing default control pipe failed: %s.\n",
		    str_error(rc));
		goto leave;
	}

	rc = usb_pipe_start_session(&dev->ctrl_pipe);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": failed to start session on control pipe: %s.\n",
		    str_error(rc));
		goto leave;
	}

	rc = usb_request_get_device_descriptor(&dev->ctrl_pipe,
	    &dev->device_descriptor);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": failed to retrieve device descriptor: %s.\n",
		    str_error(rc));
		goto leave;
	}

	rc = usb_request_get_full_configuration_descriptor_alloc(
	    &dev->ctrl_pipe, 0, (void **)&dev->full_configuration_descriptor,
	    &dev->full_configuration_descriptor_size);
	if (rc != EOK) {
		fprintf(stderr,
		    NAME ": failed to retrieve configuration descriptor: %s.\n",
		    str_error(rc));
		goto leave;
	}

	return dev;


leave:
	if (usb_pipe_is_session_started(&dev->ctrl_pipe)) {
		usb_pipe_end_session(&dev->ctrl_pipe);
	}

	free(dev);

	return NULL;
}

void destroy_device(usbinfo_device_t *dev)
{
	usb_pipe_end_session(&dev->ctrl_pipe);
	free(dev);
}

/** @}
 */
