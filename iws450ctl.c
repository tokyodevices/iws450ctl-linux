/*
 * IWS450-USB Control Program (Linux)
 * Copyright (C) 2021 Tokyo Devices, Inc.
 * https://tokyodevices.jp/
 * License: Apache License 2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include "hiddata.h"

#define USB_CFG_VENDOR_ID       0xc0, 0x16
#define USB_CFG_DEVICE_ID       0xdf, 0x05
#define USB_CFG_VENDOR_NAME     'i', 'w', '-', 't', 'e', 'c', 'h', 'f', 'i', 'r', 'm' , '.', 'c', 'o', 'm'
#define USB_CFG_DEVICE_NAME     'I', 'W', 'S' , '4', '5', '0'

#define MIN_INTERVAL_MSEC	30
#define MODE_RAW		1
#define MODE_SENS		8

#define TRUE		1
#define FALSE		0
#define HANDLE		void *

static int COUNT;
static int INTERVAL;
usbDevice_t *dev;
uint8_t buffer[17];
int err;
int counter;
HANDLE hEvent;
int recv_len = sizeof(buffer);
int mode;
int state = 0;

static char *usbErrorMessage(int errCode)
{
	static char buffer[80];

	switch(errCode)
	{
		case USBOPEN_ERR_ACCESS:      return "Access to device denied";
		case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
		case USBOPEN_ERR_IO:          return "Communication error with device";
		default:
			sprintf(buffer, "Unknown USB error %d", errCode);
			return buffer;
	}

	return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(char *serialNumber)
{
  usbDevice_t     *dev = NULL;
  unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID};
  unsigned char   rawPid[2] = {USB_CFG_DEVICE_ID};
  char            vendorName[] = {USB_CFG_VENDOR_NAME, 0};
  char		  productName[] = {USB_CFG_DEVICE_NAME, 0};
  int             vid = rawVid[0] + 256 * rawVid[1];
  int             pid = rawPid[0] + 256 * rawPid[1];
  int             err;

  if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, serialNumber, 0)) != 0){
    if( serialNumber != NULL ) {
      fprintf(stderr, "error finding %s: %s\n", serialNumber, usbErrorMessage(err));
    }
    return NULL;
  }

  return dev;
}

static void usage(char *myName)
{
	printf("IWS450-US host controller (Linux) Version 0.1.2\n");
	printf("2021-2023 (C) Tokyo Devices, Inc.\n");
	printf("https://tokyodevices.jp/\n");
	printf("usage:\n");
	printf("  %s LIST ... List all serial number of detected device(s).\n", myName);
	printf("  %s RAW <SerialNumber|\"ANY\"> <Count> <Interval=100> ... Dump raw measurement data.\n", myName);
	printf("  %s SENS <SerialNumber|\"ANY\"> <Interval=100> ... Monitor state of target device.\n", myName);
	printf("  %s SET <SerialNumber|\"ANY\"> ...  Set current value to state threshold.\n", myName);
}

static void measure(void)
{
	int count = 0;
	unsigned short average, cal;

	do
	{
		if (usbhidGetReport(dev, 0, (char *)buffer, &recv_len) != 0) {
			fprintf(stderr, "ERR\n");
			break;
		}

		average = ((buffer[4] << 8) | buffer[3]);
		cal = ((buffer[8] << 8) | buffer[7]);

		if( mode == MODE_RAW )
		{
			printf("%u\n", average);
			count++;
			if( count == COUNT ) break;
		}
		else if ( mode == MODE_SENS )
		{
			if (state == 1 && average < cal)
			{
				printf("0\n");
				state = 0;
			}
			else if (state == 0 && average >= cal)
			{
				printf("1\n");
				state = 1;
			}
		}

		usleep(INTERVAL * 1000);

	} while ( 1 );
}


int main(int argc, char **argv)
{
	if (argc < 2)
	{
		usage(argv[0]);
		exit(1);
	}

	if (strcmp(argv[1], "LIST") == 0) {
		if ((dev = openDevice(NULL)) == NULL) exit(1);

	}
	else if (strcmp(argv[1], "RAW") == 0)
	{
		mode = MODE_RAW;

		if (argc < 4)
		{
			fprintf(stderr, "Invalid arguments.\n");
			usage(argv[0]);
			exit(1);
		}
		else if (argc == 4)
		{
			INTERVAL = 100;
		}
		else
		{
			INTERVAL = atoi(argv[4]);
			if (INTERVAL < MIN_INTERVAL_MSEC)
			{
				fprintf(stderr, "WAIT must be greater than or equal to %d msec.\n", MIN_INTERVAL_MSEC);
				exit(1);
			}
		}

		COUNT = atoi(argv[3]);
		if (COUNT < 0)
		{
			fprintf(stderr, "COUNT must greater than or equal to 0.\n");
			exit(1);
		}

		if ((dev = openDevice(argv[2])) == NULL) exit(1);

		measure();
	}
	else if (strcmp(argv[1], "SENS") == 0)
	{
		mode = MODE_SENS;

		if (argc < 3) {
			fprintf(stderr, "Invalid arguments.\n");
			usage(argv[0]);
			exit(1);
		}
		else if (argc == 3)
		{
			INTERVAL = 100;
		}
		else
		{
			INTERVAL = atoi(argv[3]);
			if (INTERVAL < MIN_INTERVAL_MSEC) {
				fprintf(stderr, "WAIT must be greater than or equal to %d msec.\n", MIN_INTERVAL_MSEC);
				exit(1);
			}
		}

		if ((dev = openDevice(argv[2])) == NULL) exit(1);

		measure();
	}
	else if (strcmp(argv[1], "SET") == 0)
	{
		if (argc < 3) {
			fprintf(stderr, "Invalid arguments.\n");
			usage(argv[0]);
			exit(1);
		}

		if ((dev = openDevice(argv[2])) == NULL) exit(1);


		if ((err = usbhidGetReport(dev, 0, (char *)buffer, &recv_len)) != 0) {
			fprintf(stderr, "error read current value: %s\n", usbErrorMessage(err));
			exit(1);
		}

		unsigned short average = ((buffer[4] << 8) | buffer[3]);
		unsigned short cal = ((buffer[8] << 8) | buffer[7]);

		memset(buffer, 0, sizeof(buffer));
		buffer[0] = 0;
		buffer[1] = 0x89;
		buffer[3] = (average & 0xFF00) >> 8;
		buffer[2] = average & 0xFF;

		if ((err = usbhidSetReport(dev, (char *)buffer, sizeof(buffer))) != 0) {
			fprintf(stderr, "error set threshold value: %s\n", usbErrorMessage(err));
			exit(1);
		}

		printf("Set threshold value to %d. (previous: %d)\n", average, cal);

	}
	else if (strcmp(argv[1], "INIT") == 0) 
	{
		time_t epoc;

		memset(buffer, 0, sizeof(buffer));
		buffer[0] = 0;
		buffer[1] = 0x82;

		time(&epoc); sprintf((char *)&buffer[2],"%lu", epoc);

		if ((dev = openDevice("ANY")) == NULL) exit(1);

		if ((err = usbhidSetReport(dev, (char *)buffer, sizeof(buffer))) != 0) {
			fprintf(stderr, "error init serial: %s\n", usbErrorMessage(err));
			exit(1);
		}

		printf("Set serial number to %s\n", &buffer[2]);
	}
	else
	{
		usage(argv[0]);
		exit(1);
	}

	usbhidCloseDevice(dev);

	return 0;
}
