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

/*
 * This is mostly taken from circuitben's original software which
 * is believed to be public domain.
 *
 * http://www.circuitben.net/node/4
 */

#include "usb_492/usb492.hpp"

USB492::USB492(USB_Device *dev)
{
    _device = dev;
    _crt_control = 0x01;
}

USB492::~USB492()
{
    delete _device;
}

void USB492::write_reg(uint8_t addr, uint8_t data)
{
    _device->control(USB_Device::Control_In | USB_Device::Control_Vendor, 0x00, data, addr);
}

uint8_t USB492::read_reg(uint8_t addr)
{
    uint8_t data = 0;
    _device->control(USB_Device::Control_In | USB_Device::Control_Vendor, 0x01, 0, addr, &data, 1);
    return data;
}

void USB492::crt_page(int page)
{
    if (page == 0)
    {
        _crt_control &= ~4;
    } else {
        _crt_control |= 4;
    }
    
    write_reg(CRT_Control, _crt_control);
}

void USB492::crt_write(uint8_t pos, char ch, bool shift)
{
    // Disable display.
    // This is necessary while writing to character RAM because only one of U2024 and U1024 can drive
    // the character RAM address lines.  If you don't do this, the address when the character is written
    // will be whatever address was being scanned out at the time.
    _crt_control &= ~1;
    
    // Set address bit
    write_reg(CRT_Control, _crt_control | 2);
    
    // Write address
    write_reg(CRT_Data, pos);
    
    // Clear address bit
    write_reg(CRT_Control, _crt_control);
    
    // Write character
    uint8_t data;
    
    if (ch == ' ')
    {
        // Special case for a blank which reduces scan time.
        data = 0x80;
    } else {
        data = ch & 0x3f;
    }
    
    if (shift)
    {
        data |= 0x40;
    }
    
    write_reg(CRT_Data, data);
}

void USB492::crt_write(uint8_t pos, const char *str, bool shift)
{
    while (*str)
    {
        crt_write(pos++, *str++, shift);
    }
}

void USB492::crt_enable()
{
    _crt_control |= 0x01;
    write_reg(CRT_Control, _crt_control);
}
