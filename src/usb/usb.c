/**
 * @file usb.c
 * @desc Callback functions and endpoints for USB stack.
 * @author Samuel Dewan
 * @date 2019-01-17
 * Last Author: Samuel Dewan
 * Last Edited On: 2019-01-20
 */

// Much of this file is heavly inspired by the USB code from the Tessel firmware
// https://github.com/tessel/t2-firmware
// Some parts if this file are directly from the Tessel firmware, used under the
// MIT liscense.

#include "global.h"

#include "usb.h"
#include "class/cdc/cdc_standard.h"
#include "samd/usb_samd.h"

#include "usb-serial.h"


USB_ENDPOINTS(3);

#define INTERFACE_CDC_CONTROL   1
#define INTERFACE_CDC_DATA      2


__attribute__((__aligned__(4))) const USB_DeviceDescriptor device_descriptor = {
    .bLength = sizeof(USB_DeviceDescriptor),
    .bDescriptorType = USB_DTYPE_Device,
    
    .bcdUSB                 = 0x0200,
    .bDeviceClass           = 0,
    .bDeviceSubClass        = USB_CSCP_NoDeviceSubclass,
    .bDeviceProtocol        = USB_CSCP_NoDeviceProtocol,
    
    .bMaxPacketSize0        = 64,
    // This is Atmel's VID, with a PID set aside for use in a LUFA demo project.
    // Sorry Atmel.
    // We should consider trying to find a better VID/PID pair.
    .idVendor               = 0x03EB,
    .idProduct              = 0x2044,
    .bcdDevice              = 0x0100,   // Version Number in hundredths
    
    .iManufacturer          = 0x01,
    .iProduct               = 0x02,
    .iSerialNumber          = 0x03,
    
    .bNumConfigurations     = 1
};

// Ignore warnings about inefficient alignment
#pragma GCC diagnostic ignored "-Wpacked"

typedef struct ConfigDesc {
    USB_ConfigurationDescriptor Config;
    USB_InterfaceDescriptor OffInterface;
    
    USB_InterfaceDescriptor CDC_control_interface;
    
    CDC_FunctionalHeaderDescriptor CDC_functional_header;
    CDC_FunctionalACMDescriptor CDC_functional_ACM;
    CDC_FunctionalUnionDescriptor CDC_functional_union;
    USB_EndpointDescriptor CDC_notification_endpoint;
    
    USB_InterfaceDescriptor CDC_data_interface;
    USB_EndpointDescriptor CDC_out_endpoint;
    USB_EndpointDescriptor CDC_in_endpoint;
}  __attribute__((packed)) ConfigDesc;

// Stop ignoring warnings about inefficient alignment
#pragma GCC diagnostic pop

__attribute__((__aligned__(4))) const ConfigDesc configuration_descriptor = {
    .Config = {
        .bLength = sizeof(USB_ConfigurationDescriptor),
        .bDescriptorType = USB_DTYPE_Configuration,
        .wTotalLength  = sizeof(ConfigDesc),
        .bNumInterfaces = 3,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = USB_CONFIG_ATTR_BUSPOWERED,
        .bMaxPower = USB_CONFIG_POWER_MA(500)
    },
    .OffInterface = {
        .bLength = sizeof(USB_InterfaceDescriptor),
        .bDescriptorType = USB_DTYPE_Interface,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 0,
        .bInterfaceClass = USB_CSCP_VendorSpecificClass,
        .bInterfaceSubClass = 0x00,
        .bInterfaceProtocol = 0x00,
        .iInterface = 0,
    },
    .CDC_control_interface = {
        .bLength = sizeof(USB_InterfaceDescriptor),
        .bDescriptorType = USB_DTYPE_Interface,
        .bInterfaceNumber = INTERFACE_CDC_CONTROL,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = CDC_INTERFACE_CLASS,
        .bInterfaceSubClass = CDC_INTERFACE_SUBCLASS_ACM,
        .bInterfaceProtocol = 0,
        .iInterface = 0,
    },
    .CDC_functional_header = {
        .bLength = sizeof(CDC_FunctionalHeaderDescriptor),
        .bDescriptorType = USB_DTYPE_CSInterface,
        .bDescriptorSubtype = CDC_SUBTYPE_HEADER,
        .bcdCDC = 0x0110,
    },
    .CDC_functional_ACM = {
        .bLength = sizeof(CDC_FunctionalACMDescriptor),
        .bDescriptorType = USB_DTYPE_CSInterface,
        .bDescriptorSubtype = CDC_SUBTYPE_ACM,
        .bmCapabilities = 0x00,
    },
    .CDC_functional_union = {
        .bLength = sizeof(CDC_FunctionalUnionDescriptor),
        .bDescriptorType = USB_DTYPE_CSInterface,
        .bDescriptorSubtype = CDC_SUBTYPE_UNION,
        .bMasterInterface = INTERFACE_CDC_CONTROL,
        .bSlaveInterface = INTERFACE_CDC_DATA,
    },
    .CDC_notification_endpoint = {
        .bLength = sizeof(USB_EndpointDescriptor),
        .bDescriptorType = USB_DTYPE_Endpoint,
        .bEndpointAddress = USB_EP_CDC_NOTIFICATION,
        .bmAttributes = (USB_EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC |
                         ENDPOINT_USAGE_DATA),
        .wMaxPacketSize = 8,
        .bInterval = 0xFF
    },
    .CDC_data_interface = {
        .bLength = sizeof(USB_InterfaceDescriptor),
        .bDescriptorType = USB_DTYPE_Interface,
        .bInterfaceNumber = INTERFACE_CDC_DATA,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = CDC_INTERFACE_CLASS_DATA,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface = 0,
    },
    .CDC_out_endpoint = {
        .bLength = sizeof(USB_EndpointDescriptor),
        .bDescriptorType = USB_DTYPE_Endpoint,
        .bEndpointAddress = USB_EP_CDC_OUT,
        .bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC |
                         ENDPOINT_USAGE_DATA),
        .wMaxPacketSize = 64,
        .bInterval = 0x05
    },
    .CDC_in_endpoint = {
        .bLength = sizeof(USB_EndpointDescriptor),
        .bDescriptorType = USB_DTYPE_Endpoint,
        .bEndpointAddress = USB_EP_CDC_IN,
        .bmAttributes = (USB_EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC |
                         ENDPOINT_USAGE_DATA),
        .wMaxPacketSize = 64,
        .bInterval = 0x05
    },
};

__attribute__((__aligned__(4))) const USB_StringDescriptor language_string = {
    .bLength = USB_STRING_LEN(1),
    .bDescriptorType = USB_DTYPE_String,
    .bString = {USB_LANGUAGE_EN_US},
};

__attribute__((__aligned__(4))) uint8_t ep0_buffer[146];


// Callback on reset
void usb_cb_reset(void)
{
    
}

// Callback when a setup packet is received
void usb_cb_control_setup(void)
{
    return usb_ep0_stall();
}

// Callback on a completion interrupt
void usb_cb_completion(void)
{
    if (usb_ep_pending(USB_EP_CDC_OUT)) {
        usb_ep_handled(USB_EP_CDC_OUT);
        usb_serial_out_completion();
    }
    
    if (usb_ep_pending(USB_EP_CDC_IN)) {
        usb_ep_handled(USB_EP_CDC_IN);
        usb_serial_in_completion();
    }
}
void usb_cb_control_in_completion(void)
{
    
}
void usb_cb_control_out_completion(void)
{
    
}

// Callback for a SET_CONFIGURATION request
bool usb_cb_set_configuration(uint8_t config)
{
    if (config <= 1) {
        usb_serial_init();
        return true;
    }
    return false;
}

// Callback for a SET_INTERFACE request
bool usb_cb_set_interface(uint16_t interface, uint16_t altsetting)
{
    return false;
}

// Callbck for a GET_DESCRIPTOR request
uint16_t usb_cb_get_descriptor(uint8_t type, uint8_t index,
                               const uint8_t** descriptor_ptr)
{
    const void* address = NULL;
    uint16_t size    = 0;
    
    switch (type) {
        case USB_DTYPE_Device:
            address = &device_descriptor;
            size    = sizeof(USB_DeviceDescriptor);
            break;
        case USB_DTYPE_Configuration:
            address = &configuration_descriptor;
            size    = sizeof(ConfigDesc);
            break;
        case USB_DTYPE_String:
            switch (index) {
                case 0x00:
                    address = &language_string;
                    break;
                case 0x01:
                    address = usb_string_to_descriptor("CU InSpace");
                    break;
                case 0x02:
                    address = usb_string_to_descriptor("CU InSpace 2019 MCU "
                                                       "Board");
                    break;
                case 0x03:
                    address = samd_serial_number_string_descriptor();
                    break;
            }
            size = (((const USB_StringDescriptor*)address))->bLength;
            break;
    }
    
    *descriptor_ptr = address;
    return size;
}
