#ifndef USB_H
#define USB_H
typedef unsigned char		uchar;

#include <part.h>

int usb_init(void); /* initialize the USB Controller */
void usb_stor_reset();
int usb_stor_scan(int mode);
block_dev_desc_t *usb_stor_get_dev(int index);
int usb_stop(void);
void rtk_usb_power_off(void);

#endif