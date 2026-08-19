/*
 * USBDriver.c
 *
 *  Created on: Aug 18, 2026
 *      Author: 310004
 */

#include "USBDriver.h"


//void onUsbReceive2(uint8_t* buf, uint32_t len)
//{
//	extern QueueHandle_t usbRxQueue;
//
//    uint8_t localBuf[64];
//    size_t copyLen = (len > 64) ? 64 : len;
//    memcpy(localBuf, buf, copyLen);
//
//    BaseType_t hpw = pdFALSE;
//    xQueueSendFromISR(usbRxQueue, localBuf, &hpw);
//    portYIELD_FROM_ISR(hpw);
//}

