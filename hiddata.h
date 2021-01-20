#ifndef __HIDDATA_H__
#define __HIDDATA_H__

#define USBOPEN_SUCCESS         0
#define USBOPEN_ERR_ACCESS      1
#define USBOPEN_ERR_IO          2
#define USBOPEN_ERR_NOTFOUND    3

typedef struct usbDevice    usbDevice_t;

int usbhidOpenDevice(usbDevice_t **device, int vendor, char *vendorName, int product, char *productName, char *serialNumber, int _usesReportIDs);
void    usbhidCloseDevice(usbDevice_t *device);
int usbhidSetReport(usbDevice_t *device, char *buffer, int len);
int usbhidGetReport(usbDevice_t *device, int reportID, char *buffer, int *len);

#endif
