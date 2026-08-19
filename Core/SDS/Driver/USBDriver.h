/*
 * USBDriver.h
 *
 *  Created on: Aug 18, 2026
 *      Author: 310004
 */
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include <stdbool.h>
#include "usbd_cdc_if.h"

static bool USB_SendDetection(uint32_t timestamp, uint32_t micId, float azimuth, float distance, float confidence)
{
    struct DetectionMsg {
        uint32_t magic;
        uint32_t ts;
        uint32_t mic;
        float    azi;
        float    distance;
        float    conf;
    } msg;

    msg.magic 	 = 0xDEADBEEF;
    msg.ts    	 = timestamp;
    msg.mic   	 = micId;
    msg.azi   	 = azimuth;
    msg.distance = distance;
    msg.conf  	 = confidence;

    uint8_t status =  CDC_Transmit_FS((uint8_t*)(&msg), sizeof(msg));
    return (status == USBD_OK);
}


//void onUsbReceive2(uint8_t* buf, uint32_t len);
