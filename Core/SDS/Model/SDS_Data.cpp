/*
 * SDS_Data.cpp
 *
 * System Integration Description
 * ------------------------------
 * SDS_Data implements the synchronized global data model of the SDS
 * (Sensor‑DSP‑System). It provides deterministic, thread‑safe access to
 * all real‑time DSP results such as:
 *
 *      - azimuth (degrees)
 *      - distance (meters)
 *      - confidence (0…1)
 *      - detection flag
 *      - debug values
 *
 * Thread‑Safety Model:
 * --------------------
 * All read/write operations are protected by a CMSIS‑RTOS2 mutex:
 *
 *      osMutexAcquire(mutex, osWaitForever);
 *      ... update or read shared state ...
 *      osMutexRelease(mutex);
 *
 * This ensures deterministic behavior under concurrent access from:
 *
 *   - SRPTask (azimuth + distance)
 *   - MicTask (confidence + debug values)
 *   - LCDTask (display reads)
 *
 * Event Queue:
 * ------------
 * SDS_Data also provides an RTOS message queue for asynchronous events.
 * Each write operation pushes an event:
 *
 *      pushEvent(SDS_DataEventType::SRP_UPDATE, processTime);
 *
 * This allows the DisplayManager or other tasks to react to DSP updates
 * without polling.
 *
 * Created on: Jun 18, 2026
 * Author: Stefan (310004)
 */

#include "SDS_Data.hpp"

// ---------------------------------------------------------------------------
// Singleton instance
// ---------------------------------------------------------------------------
SDS_Data& SDS_Data::instance()
{
    static SDS_Data instance;
    return instance;
}

// ---------------------------------------------------------------------------
// Constructor: initializes mutex and event queue.
// ---------------------------------------------------------------------------
SDS_Data::SDS_Data()
    : mutex(nullptr),
      azimuthDeg(0.0f),
      distance(0.0f),
      confidence(0.0f),
      detected(false),
      debugTime(0.0f),
      debugValue(0.0f),
      debugValue1(0.0f),
      debugValue2(0.0f),
      debugValue3(0.0f),
      eventQueue(nullptr)
{
    // Create mutex for thread‑safe access
    mutex = osMutexNew(nullptr);

    // Create event queue (16 messages, each SDS_DataEvent)
    const osMessageQueueAttr_t qAttr = { };
    eventQueue = osMessageQueueNew(16, sizeof(SDS_DataEvent), &qAttr);
}

// ---------------------------------------------------------------------------
// Push event into RTOS queue
// ---------------------------------------------------------------------------
void SDS_Data::pushEvent(SDS_DataEventType type, float processTime)
{
    if (!eventQueue) {
        return;
    }

    SDS_DataEvent evt;
    evt.type        = type;
    evt.processTime = processTime;

    (void)osMessageQueuePut(eventQueue, &evt, 0, 0);
}

// ---------------------------------------------------------------------------
// Write API — DSP tasks update shared state
// ---------------------------------------------------------------------------
void SDS_Data::setAzimuth(float az)
{
    osMutexAcquire(mutex, osWaitForever);
    azimuthDeg = az;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::SRP_UPDATE, 0.0f);
}

void SDS_Data::setDistance(float d)
{
    osMutexAcquire(mutex, osWaitForever);
    distance = d;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::SRP_UPDATE, 0.0f);
}

void SDS_Data::setConfidence(float c)
{
    osMutexAcquire(mutex, osWaitForever);
    confidence = c;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::SRP_UPDATE, 0.0f);
}

void SDS_Data::setDetected(bool d)
{
    osMutexAcquire(mutex, osWaitForever);
    detected = d;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::SRP_UPDATE, 0.0f);
}

// ---------------------------------------------------------------------------
// Debug values
// ---------------------------------------------------------------------------
void SDS_Data::setDebugTime(float ms)
{
    osMutexAcquire(mutex, osWaitForever);
    debugTime = ms;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::DEBUG_UPDATE, 0.0f);
}

void SDS_Data::setDebugValue(float val)
{
    osMutexAcquire(mutex, osWaitForever);
    debugValue = val;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::DEBUG_UPDATE, 0.0f);
}

void SDS_Data::setDebugValue1(float val)
{
    osMutexAcquire(mutex, osWaitForever);
    debugValue1 = val;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::DEBUG_UPDATE, 0.0f);
}

void SDS_Data::setDebugValue2(float val)
{
    osMutexAcquire(mutex, osWaitForever);
    debugValue2 = val;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::DEBUG_UPDATE, 0.0f);
}

void SDS_Data::setDebugValue3(float val)
{
    osMutexAcquire(mutex, osWaitForever);
    debugValue3 = val;
    osMutexRelease(mutex);

    pushEvent(SDS_DataEventType::DEBUG_UPDATE, 0.0f);
}

// ---------------------------------------------------------------------------
// Read API — DisplayManager reads shared state
// ---------------------------------------------------------------------------
float SDS_Data::getAzimuth() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = azimuthDeg;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDistance() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = distance;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getConfidence() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = confidence;
    osMutexRelease(mutex);
    return v;
}

bool SDS_Data::getDetected() const
{
    osMutexAcquire(mutex, osWaitForever);
    bool v = detected;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDebugTime() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = debugTime;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDebugValue() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = debugValue;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDebugValue1() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = debugValue1;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDebugValue2() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = debugValue2;
    osMutexRelease(mutex);
    return v;
}

float SDS_Data::getDebugValue3() const
{
    osMutexAcquire(mutex, osWaitForever);
    float v = debugValue3;
    osMutexRelease(mutex);
    return v;
}
