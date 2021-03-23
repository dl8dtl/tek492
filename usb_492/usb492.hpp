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


#ifndef _USB492_HPP_
#define _USB492_HPP_

#include "USB_Device.hpp"

class USB492
{
public:
    // Addresses
    static const uint8_t CRT_Data           = 0x2f;
    static const uint8_t CRT_Control        = 0x5f;
    
    static const uint8_t Storage_Data       = 0x7a;
    static const uint8_t Storage_Control    = 0x7b;
    
    USB492(USB_Device *dev);
    ~USB492();

    void write_reg(uint8_t addr, uint8_t data);
    uint8_t read_reg(uint8_t addr);
    
    void crt_page(int page);
    void crt_write(uint8_t pos, char ch, bool shift = false);
    void crt_write(uint8_t pos, const char *str, bool shift = false);
    void crt_enable();

protected:
    USB_Device *_device;
    uint8_t _crt_control;
};

#endif // _USB492_HPP_
