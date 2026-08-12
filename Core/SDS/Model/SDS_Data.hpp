/*
 * SDS_Data.hpp
 *
 * Mathematical / Physical Description
 * -----------------------------------
 * SDS_Data is the central shared data model of the SDS (Sensor‑DSP‑System).
 * It provides deterministic, thread‑safe access to all real‑time DSP results
 * such as:
 *
 *      - azimuth (degrees)
 *      - distance (meters)
 *      - confidence (0…1)
 *      - detection flag
 *      - debug values
 *
 * The class acts as a synchronized state container between:
 *
 *   - DSP‑producing tasks:
 *         SRPTask, MicTask, DSPTask
 *
 *   - DSP‑consuming tasks:
 *         LCDTask (DisplayManager)
 *
 * Thread‑Safety Model:
 * --------------------
 * All write and read operations are protected by a CMSIS‑RTOS2 mutex:
 *
 *      lock(mutex)
 *      update or read shared state
 *      unlock(mutex)
 *
 * This ensures deterministic behavior under concurrent access and prevents
 * race conditions between high‑frequency DSP updates and lower‑frequency
 * visualization tasks.
 *
 * Event Queue:
 * ------------
 * SDS_Data also provides an RTOS message queue for asynchronous events:
 *
 *      pushEvent(type, processTime)
 *
 * Events allow tasks to signal:
 *   - SRP updates
 *   - DSP timing information
 *   - detection changes
 *
 * Timing Model:
 * -------------
 * The data model is updated periodically by DSP tasks with sampling period:
 *
 *      T = delayMs / 1000 seconds
 *
 * ensuring consistent real‑time behavior.
 *
 * Created on: Jun 18, 2026
 * Author: Stefan (310004)
 */

#pragma once

#include <cstdint>
#include "cmsis_os.h"
#include "string.h"
#include "SDS_Params.hpp"
#include "SDS_Structs.hpp"

class SDS_Data {
public:
    // Singleton instance — ensures a single global data model.
    static SDS_Data& instance();

    // ---------------------------------------------------------------------
    // Write API (called by DSPTask, SRPTask, MicTask, SystemManager)
    // ---------------------------------------------------------------------
    void setAzimuth(float az);
    void setDistance(float d);
    void setConfidence(float c);
    void setDetected(bool d);

    void setDebugTime(float ms);
    void setDebugValue(float val);
    void setDebugValue1(float val);
    void setDebugValue2(float val);
    void setDebugValue3(float val);

    // ---------------------------------------------------------------------
    // Read API (called by DisplayManager / LCDTask)
    // ---------------------------------------------------------------------
    float getAzimuth() const;
    float getDistance() const;
    float getConfidence() const;
    bool  getDetected() const;

    float getDebugTime() const;
    float getDebugValue() const;
    float getDebugValue1() const;
    float getDebugValue2() const;
    float getDebugValue3() const;

    // ---------------------------------------------------------------------
    // Event Queue API
    // ---------------------------------------------------------------------
    osMessageQueueId_t getEventQueue() const { return eventQueue; }

    // Pushes an event into the RTOS queue (non‑blocking).
    void pushEvent(SDS_DataEventType type, float processTime);

private:
    // Constructor: initializes mutex and queue.
    SDS_Data();

    // Mutex for thread‑safe access.
    mutable osMutexId_t mutex;

    // Core DSP values.
    float azimuthDeg;
    float distance;
    float confidence;
    bool  detected;

    // Debug values (temporary, can be extended to arrays).
    float debugTime;
    float debugValue;
    float debugValue1;
    float debugValue2;
    float debugValue3;

    // RTOS event queue for asynchronous notifications.
    osMessageQueueId_t eventQueue;
};
