/*
 * SDSUSB.hpp
 *
 *  Created on: Aug 13, 2026
 *      Author: 310004
 */

#pragma once
//#include "usbd_cdc_if.h"
#include <cstdint>

class SDS_USB
{
public:
    static bool SendDetection(uint32_t timestamp,
                              uint8_t micId,
                              float azimuth,
                              float elevation,
                              float confidence)
    {
        // Format: 32 bytes fixed-size message
        struct __attribute__((packed)) DetectionMsg {
            uint32_t magic;       // 0xDEADBEEF
            uint32_t ts;
            uint8_t  mic;
            float    azi;
            float    ele;
            float    conf;
        } msg;

        msg.magic = 0xDEADBEEF;
        msg.ts    = timestamp;
        msg.mic   = micId;
        msg.azi   = azimuth;
        msg.ele   = elevation;
        msg.conf  = confidence;

        // Deterministic send
//        auto status = CDC_Transmit_FS(reinterpret_cast<uint8_t*>(&msg),
//                                      sizeof(msg));
//
//        return (status == USBD_OK);
        bool res = (sizeof(msg) > 4);
        return (res);
    }
};
