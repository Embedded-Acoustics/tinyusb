/*
 * test_hidh_mouse.c
 *
 *  Created on: Apr 6, 2013
 *      Author: hathach
 */

/*
 * Software License Agreement (BSD License)
 * Copyright (c) 2012, hathach (tinyusb.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the tiny usb stack.
 */

#include "stdlib.h"
#include "unity.h"
#include "type_helper.h"
#include "errors.h"
#include "hid_host.h"
#include "mock_osal.h"
#include "mock_usbh.h"
#include "mock_hcd.h"
#include "descriptor_test.h"

extern hidh_interface_info_t mouse_data[TUSB_CFG_HOST_DEVICE_MAX];
hidh_interface_info_t *p_hidh_mouse;

tusb_descriptor_interface_t const *p_mouse_interface_desc = &desc_configuration.mouse_interface;
tusb_descriptor_endpoint_t  const *p_mouse_endpoint_desc  = &desc_configuration.mouse_endpoint;

uint8_t dev_addr;

void setUp(void)
{
  hidh_init();

  dev_addr = RANDOM(TUSB_CFG_HOST_DEVICE_MAX)+1;

  p_hidh_mouse = &mouse_data[dev_addr-1];

  p_hidh_mouse->report_size = sizeof(tusb_mouse_report_t);
  p_hidh_mouse->pipe_hdl = (pipe_handle_t) {
    .dev_addr  = dev_addr,
    .xfer_type = TUSB_XFER_INTERRUPT,
    .index     = 1
  };
}

void tearDown(void)
{

}

void test_mouse_init(void)
{
  hidh_init();

  TEST_ASSERT_MEM_ZERO(mouse_data, sizeof(hidh_interface_info_t)*TUSB_CFG_HOST_DEVICE_MAX);
}

//------------- is supported -------------//
void test_mouse_is_supported_fail_unplug(void)
{
  tusbh_device_get_state_IgnoreAndReturn(TUSB_DEVICE_STATE_UNPLUG);
  TEST_ASSERT_FALSE( tusbh_hid_mouse_is_supported(dev_addr) );
}

void test_mouse_is_supported_fail_not_opened(void)
{
  hidh_init();
  tusbh_device_get_state_IgnoreAndReturn(TUSB_DEVICE_STATE_CONFIGURED);
  TEST_ASSERT_FALSE( tusbh_hid_mouse_is_supported(dev_addr) );
}

void test_mouse_is_supported_ok(void)
{
  tusbh_device_get_state_IgnoreAndReturn(TUSB_DEVICE_STATE_CONFIGURED);
  TEST_ASSERT_TRUE( tusbh_hid_mouse_is_supported(dev_addr) );
}

void test_mouse_open_ok(void)
{
  uint16_t length=0;
  pipe_handle_t pipe_hdl = {.dev_addr = dev_addr, .xfer_type = TUSB_XFER_INTERRUPT, .index = 2};

  hidh_init();

  hcd_pipe_open_ExpectAndReturn(dev_addr, p_mouse_endpoint_desc, TUSB_CLASS_HID, pipe_hdl);

  //------------- Code Under TEST -------------//
  TEST_ASSERT_EQUAL(TUSB_ERROR_NONE, hidh_open_subtask(dev_addr, p_mouse_interface_desc, &length));

  TEST_ASSERT_PIPE_HANDLE(pipe_hdl, p_hidh_mouse->pipe_hdl);
  TEST_ASSERT_EQUAL(8, p_hidh_mouse->report_size);
  TEST_ASSERT_EQUAL(sizeof(tusb_descriptor_interface_t) + sizeof(tusb_hid_descriptor_hid_t) + sizeof(tusb_descriptor_endpoint_t),
                    length);

  tusbh_device_get_state_IgnoreAndReturn(TUSB_DEVICE_STATE_CONFIGURED);
  TEST_ASSERT_TRUE( tusbh_hid_keyboard_is_supported(dev_addr) );
  TEST_ASSERT_EQUAL(TUSB_INTERFACE_STATUS_READY, p_hidh_mouse->status);

}


