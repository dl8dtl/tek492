/*
 * This is mostly taken from circuitben's original software which
 * is believed to be public domain.
 *
 * http://www.circuitben.net/node/4
 */

/*
 * As this application uses Qt, any redistribution is bound to the
 * terms of either the GNU Lesser General Public License v. 3 ("LGPL"),
 * or to any commercial Qt license.
 */

/*
 * This application uses the Qwt extensions to Qt, which are also
 * provided under the terms of LGPL v2.1.  However, the Qwt license
 * explicitly disclaims many common use cases as not constituting a
 * "derived work" (in the LGPL sense), so it is generally less
 * demanding about the redistribution policy than Qt itself.
 */

/* $Id$ */

#ifndef _USB_DEVICE_HPP_
#define _USB_DEVICE_HPP_

#include <stdint.h>
#ifdef _WIN32
#  include <lusb0_usb.h>
#else
#  include <usb.h>
#endif
#include <vector>

class USB_Device
{
public:
    static USB_Device *find_first(uint16_t vendor, uint16_t product);
    static void find_all(std::vector<USB_Device *> &devs, uint16_t vendor, uint16_t product);
    
    ~USB_Device();

    bool open();

    bool set_configuration(int n);
    bool set_altinterface(int n);
    bool claim_interface(int n);
    bool release_interface(int n);
    
    // Sets the first available configuration, interface, and
    // alternate interface.
    bool set_default();
    
    // Control message type bits: pick one from each group
    // Direction
    static const uint8_t Control_In = 0x80;     // Device to host
    static const uint8_t Control_Out = 0x00;    // Host to device
    // Who defines the request
    static const uint8_t Control_Standard = 0 << 5;
    static const uint8_t Control_Class = 1 << 5;
    static const uint8_t Control_Vendor = 2 << 5;
    // Recipient of the request
    static const uint8_t Control_Device = 0;
    static const uint8_t Control_Interface = 1;
    static const uint8_t Control_Endpoint = 2;
    static const uint8_t Control_Other = 3;
    
    // Sends a control message.
    // Whether data is read or written is determined by the Control_Out/Control_In type bit.
    bool control(uint8_t type, uint8_t request, uint16_t value, uint16_t index, void *data = 0, int size = 0, int timeout = 0, int *bytes_done = 0);
    
    bool bulk_write(int endpoint, void *data, unsigned int size, int timeout = 0, unsigned int *bytes_done = 0);
    bool bulk_read(int endpoint, void *data, unsigned int size, int timeout = 0, unsigned int *bytes_done = 0);

protected:
    static void init();
    
    USB_Device(struct usb_device *dev);
    
    static bool inited;
    
    struct usb_device *_dev;
    usb_dev_handle *_handle;
};

#endif // _USB_DEVICE_HPP_
