/*
 * SDS_MicrophoneBuffer.cpp
 *
 * Real‑Time Buffering Description
 * -------------------------------
 * This file implements the deterministic triple‑buffer system used for
 * microphone acquisition in the SDS (Sensor‑DSP‑System). The buffer model
 * ensures that:
 *
 *   - MicTask (producer) can continuously write audio blocks
 *   - SRPTask / DSPTask (consumer) can safely read complete blocks
 *   - no buffer is overwritten while being processed
 *   - no DSP cycle blocks microphone acquisition
 *
 * Buffering Model:
 * ----------------
 * Three MicBuffer instances rotate through the following states:
 *
 *      BUF_FREE    → available for writing
 *      BUF_WRITING → acquisition task is filling the buffer
 *      BUF_READY   → buffer contains a complete frame
 *      BUF_READING → DSP task is processing the buffer
 *
 * Only one buffer may be in BUF_READY at any time. When a new buffer is
 * marked ready, all other ready buffers are reset to BUF_FREE to maintain
 * deterministic behavior.
 *
 * Thread‑Safety:
 * --------------
 * All state transitions are protected by a CMSIS‑RTOS2 mutex to prevent
 * race conditions between producer and consumer tasks.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#include <SDS_MicrophoneBuffer.hpp>

// ---------------------------------------------------------------------------
// Singleton instance
// ---------------------------------------------------------------------------
SDS_MicrophoneBuffer& SDS_MicrophoneBuffer::instance()
{
    static SDS_MicrophoneBuffer instance;
    return instance;
}

// ---------------------------------------------------------------------------
// Constructor: initialize mutex and buffer states
// ---------------------------------------------------------------------------
SDS_MicrophoneBuffer::SDS_MicrophoneBuffer()
{
    mutex = osMutexNew(nullptr);

    for (int i = 0; i < 3; i++) {
        buffers[i].state = BUF_FREE;
    }
}

// ---------------------------------------------------------------------------
// Acquire a free buffer for writing (MicTask)
// ---------------------------------------------------------------------------
MicBuffer* SDS_MicrophoneBuffer::getFreeBuffer()
{
    for (int i = 0; i < 3; i++) {
        if (buffers[i].state == BUF_FREE) {
            return &buffers[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Acquire a ready buffer for reading (SRPTask / DSPTask)
// ---------------------------------------------------------------------------
MicBuffer* SDS_MicrophoneBuffer::getReadableBuffer()
{
    for (int i = 0; i < 3; i++) {
        if (buffers[i].state == BUF_READY) {
            return &buffers[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Mark buffer as ready after writing
// Ensures only one buffer is in BUF_READY at any time.
// ---------------------------------------------------------------------------
void SDS_MicrophoneBuffer::markReadable(MicBuffer* b)
{
    osMutexAcquire(mutex, osWaitForever);

    b->state = BUF_READY;

    // Ensure only one readable buffer exists
    for (int i = 0; i < 3; i++) {
        if (&buffers[i] != b && buffers[i].state == BUF_READY) {
            buffers[i].state = BUF_FREE;
        }
    }

    osMutexRelease(mutex);
}

// ---------------------------------------------------------------------------
// Mark buffer as free after DSP processing
// ---------------------------------------------------------------------------
void SDS_MicrophoneBuffer::markFree(MicBuffer* b)
{
    osMutexAcquire(mutex, osWaitForever);
    b->state = BUF_FREE;
    osMutexRelease(mutex);
}
