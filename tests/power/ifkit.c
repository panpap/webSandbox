#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <phidget21.h>

#include "common.h"
#include "ifkit.h"

#define CURRENT_12V	2 /* yellow line, to cpu and pci express bus */
#define CURRENT_5V	0 /* red line, to memory */
#define CURRENT_3_3V	4 /* orange line, to low power motherboard peripherals */
#define CURRENT_MISC	6 /* extra line, to discrete gpu for example */

/* these are unused and considered static */
#define VOLTAGE_12V	3
#define VOLTAGE_5V	1
#define VOLTAGE_3_3V	5
#define VOLTAGE_MISC	7

/* comment this out to get real voltage values */
#define STATIC_VOLTAGE

/* formulas from:
   http://www.phidgets.com/docs/1122_User_Guide */
#define DC_CURRENT_FORMULA(adval) (((double)adval / 13.2F) - 37.6787F)
#define AC_CURRENT_FORMULa(adval) ((double)adval * 0.04204F)
#define DC_VOLTAGE_FORMULA(adval) ((adval * 0.06F) - 30)

/* CCONV is needed for the windows calling convention */

struct power_stats p_12v, p_5v, p_3_3v, p_misc;

void
display_generic_properties(CPhidgetHandle phid)
{
	int sernum, version;
	const char *deviceptr;
	CPhidget_getDeviceType(phid, &deviceptr);
	CPhidget_getSerialNumber(phid, &sernum);
	CPhidget_getDeviceVersion(phid, &version);

	printf("%s\n", deviceptr);
	printf("Version: %8d SerialNumber: %10d\n", version, sernum);
	return;
}


int CCONV
IFK_DetachHandler(CPhidgetHandle phid, void *userptr)
{
	printf("Detach handler ran!\n");
	return 0;
}

int CCONV
IFK_ErrorHandler(CPhidgetHandle phid, void *userptr, int ErrorCode, const char *unknown)
{
	printf("Error handler ran!\n");
	return 0;
}

int CCONV
IFK_OutputChangeHandler(CPhidgetInterfaceKitHandle phid, void *userptr, int idx, int val)
{
	printf("Output %d is %d\n", idx, val);
	return 0;
}

int CCONV
IFK_InputChangeHandler(CPhidgetInterfaceKitHandle phid, void *userptr, int idx, int val)
{
	printf("Input %d is %d\n", idx, val);
	return 0;
}

int CCONV
IFK_SensorChangeHandler(CPhidgetInterfaceKitHandle phid, void *userptr, int idx, int val)
{
	DPRINTF_D(idx);
	DPRINTF_D(val);

	switch (idx) {
	case CURRENT_12V:
		p_12v.current = DC_CURRENT_FORMULA(val);
		break;
	case CURRENT_5V:
		p_5v.current = DC_CURRENT_FORMULA(val);
		break;
	case CURRENT_3_3V:
		p_3_3v.current = DC_CURRENT_FORMULA(val);
		break;
	case CURRENT_MISC:
		p_misc.current = DC_CURRENT_FORMULA(val);
		break;
#ifndef STATIC_VOLTAGE
	case VOLTAGE_12V:
		p_12v.voltage = DC_VOLTAGE_FORMULA(val);
		break;
	case VOLTAGE_5V:
		p_5v.voltage = DC_VOLTAGE_FORMULA(val);
		break;
	case VOLTAGE_3_3V:
		p_3_3v.voltage = DC_VOLTAGE_FORMULA(val);
		break;
	case VOLTAGE_MISC:
		p_misc.voltage = DC_VOLTAGE_FORMULA(val);
		break;
#else /* STATIC_VOLTAGE */
	case VOLTAGE_12V:
		p_12v.voltage = 12.0F;
		break;
	case VOLTAGE_5V:
		p_5v.voltage = 5.0F;
		break;
	case VOLTAGE_3_3V:
		p_3_3v.voltage = 3.3F;
		break;
	case VOLTAGE_MISC:
		p_misc.voltage = 12.0F;
		break;
#endif
	default:
		ERRXV(1, "wrong index: %d", idx);
	}

	p_12v.power = p_12v.voltage * p_12v.current;
	p_5v.power = p_5v.voltage * p_5v.current;
	p_3_3v.power = p_3_3v.voltage * p_3_3v.current;
	p_misc.power = p_misc.voltage * p_misc.current;

	p_12v.sum_power += p_12v.power;
	p_5v.sum_power += p_5v.power;
	p_3_3v.sum_power += p_3_3v.power;
	p_misc.sum_power += p_misc.power;

	p_5v.samples++;
	p_12v.samples++;
	p_3_3v.samples++;	
	p_misc.samples++;

	p_12v.mean_power = p_12v.sum_power / (double)p_12v.samples;
	p_5v.mean_power = p_5v.sum_power / (double)p_5v.samples;
	p_3_3v.mean_power = p_3_3v.sum_power / (double)p_3_3v.samples;
	p_misc.mean_power = p_misc.sum_power / (double)p_misc.samples;

	return 0;
}


int CCONV
IFK_AttachHandler(CPhidgetHandle phid, void *userptr)
{
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 0, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 1, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 2, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 3, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 4, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 5, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 6, 0);
	CPhidgetInterfaceKit_setSensorChangeTrigger((CPhidgetInterfaceKitHandle)phid, 7, 0);
	return 0;
}

/* initialize sensor interface */
int
ifkit_init(void)
{
	int numInputs, numOutputs, numSensors;
	int err;
	CPhidgetInterfaceKitHandle phid = 0;

/*
	CPhidget_enableLogging(PHIDGET_LOG_VERBOSE, NULL);
*/
	
	CPhidgetInterfaceKit_create(&phid);

	CPhidgetInterfaceKit_set_OnSensorChange_Handler(phid, IFK_SensorChangeHandler, NULL);
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)phid, IFK_AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)phid, IFK_DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)phid, IFK_ErrorHandler, NULL);
	
	CPhidget_open((CPhidgetHandle)phid, -1);

	/* wait 5 seconds for attachment */
	err = CPhidget_waitForAttachment((CPhidgetHandle)phid, 5000);
	if (err != EPHIDGET_OK) {
		const char *errstr;
		CPhidget_getErrorDescription(err, &errstr);
		fprintf(stderr, "did not attach: %s\n", errstr);
		CPhidget_close((CPhidgetHandle)phid);
		CPhidget_delete((CPhidgetHandle)phid);
		return 1;
	}
	
	/* this is about callibrating the voltage sensor */
	CPhidgetInterfaceKit_setRatiometric((CPhidgetInterfaceKitHandle)phid, 0);

	return 0;
}

/* clear power stats */
void
ifkit_clear(void)
{
	p_12v.sum_power = 0;
	p_12v.samples = 1;
	p_5v.sum_power = 0;
	p_5v.samples = 1;
	p_3_3v.sum_power = 0;
	p_3_3v.samples = 1;
	p_misc.sum_power = 0;
	p_misc.samples = 1;    
}
