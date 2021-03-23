#include <stdio.h>

#include "usb492.hpp"

using namespace std;

USB_Device *dev = 0;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: test <address in hex> [<data in hex>]\n");
        return 1;
    }
    
    vector<USB_Device *> devs;
    USB_Device::find_all(devs, 0x3141, 0x0005);
    
    if (devs.empty())
    {
        printf("USB492 device not found\n");
        return 1;
    }
    
    int n = 0;
    dev = devs[n];
    if (!dev->open())
    {
        printf("Can't open\n");
        return 1;
    }
    if (!dev->set_default())
    {
        printf("Can't set default config\n");
        return 1;
    }
    
    USB492 *usb492 = new USB492(dev);
    
    uint8_t addr = strtol(argv[1], 0, 16);
    
    if (argc == 3)
    {
        uint8_t data = strtol(argv[2], 0, 16);
        usb492->write_reg(addr, data);
    } else {
        uint8_t data = usb492->read_reg(addr);
        printf("0x%02x\n", data);
    }
    
    delete usb492;
    
    return 0;
}
