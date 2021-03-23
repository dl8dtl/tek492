#include <stdio.h>
#include <unistd.h>

#include "usb492.hpp"

using namespace std;

USB_Device *dev = 0;

int main(int argc, char *argv[])
{
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
    
    // Halt acquisition
    usb492->write_reg(USB492::Storage_Control, 0x66);
    usleep(5 * 1000);
    
    // Discard bogus samples at the beginning
    uint8_t buf[1024];
    dev->control(USB_Device::Control_In | USB_Device::Control_Vendor, 0x01, 0, USB492::Storage_Data, buf, 1024);
    for (int i = 4; i <= 1018; ++i)
    {
        printf("%d\n", buf[i]);
    }
    
    // Resume acquisition
    usb492->write_reg(USB492::Storage_Control, 0xe6);
    
    delete usb492;
    
    return 0;
}
