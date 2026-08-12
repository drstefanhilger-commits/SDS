/*
 * SDS_MicrophoneBuffer.hpp
 *
 * Real‑Time Buffering Description
 * -------------------------------
 * SDS_MicrophoneBuffer implements the deterministic triple‑buffer system
 * used for microphone data acquisition in the SDS (Sensor‑DSP‑System).
 *
 * The buffer model is designed for:
 *   - 8‑channel microphone arrays (ADAU7118)
 *   - fixed block size SDS_BLOCK_SIZE
 *   - real‑time DSP processing (SRP‑PHAT, DAS, FFT)
 *   - zero‑copy handoff between acquisition and DSP tasks
 *
 * Buffering Model:
 * ----------------
 * Three MicBuffer instances are used in a rotating fashion:
 *
 *      buffers[0]   buffers[1]   buffers[2]
 *
 * Each buffer has a state:
 *
 *      BUF_FREE      : available for writing
 *      BUF_WRITING   : acquisition task is filling the buffer
 *      BUF_READY     : buffer contains a complete frame
 *      BUF_READING   : DSP task is processing the buffer
 *
 * This ensures:
 *   - no overwriting of data currently being processed
 *   - no blocking of acquisition due to DSP load
 *   - deterministic timing under RTOS scheduling
 *
 * Thread‑Safety:
 * --------------
 * All buffer state transitions are protected by a CMSIS‑RTOS2 mutex.
 * This prevents race conditions between:
 *
 *   - MicTask (producer)
 *   - SRPTask / DSPTask (consumer)
 *
 * Timing Model:
 * -------------
 * The acquisition task fills one buffer per DSP cycle:
 *
 *      T = SDS_BLOCK_SIZE / sampleRate
 *
 * The DSP task consumes one buffer per runOnce() cycle.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>
#include "cmsis_os2.h"
#include "SDS_Params.hpp"

// ---------------------------------------------------------------------------
// Buffer states for deterministic triple‑buffering
// ---------------------------------------------------------------------------
enum BufferState : uint8_t {
    BUF_FREE    = 0,   // buffer available for writing
    BUF_WRITING = 1,   // acquisition task is filling the buffer
    BUF_READY   = 2,   // buffer contains a complete frame
    BUF_READING = 3    // DSP task is processing the buffer
};

// ---------------------------------------------------------------------------
// 8‑channel microphone buffer
// ---------------------------------------------------------------------------
struct MicBuffer {
    BufferState state;                                 // current buffer state
    float data[SDS_NUM_MICS][SDS_BLOCK_SIZE];          // microphone samples
};

// ---------------------------------------------------------------------------
// Triple‑buffer manager
// ---------------------------------------------------------------------------
class SDS_MicrophoneBuffer
{
public:
    // Singleton instance — ensures a single buffer manager exists.
    static SDS_MicrophoneBuffer& instance();

    // Acquire a free buffer for writing (MicTask)
    MicBuffer* getFreeBuffer();

    // Acquire a ready buffer for reading (SRPTask / DSPTask)
    MicBuffer* getReadableBuffer();

    // Mark buffer as ready after writing
    void markReadable(MicBuffer* b);

    // Mark buffer as free after DSP processing
    void markFree(MicBuffer* b);

private:
    // Constructor: initializes buffer states and mutex
    SDS_MicrophoneBuffer();

    // Disable copy operations (singleton)
    SDS_MicrophoneBuffer(const SDS_MicrophoneBuffer&) = delete;
    SDS_MicrophoneBuffer& operator=(const SDS_MicrophoneBuffer&) = delete;

private:
    MicBuffer buffers[3];      // triple‑buffer array
    osMutexId_t mutex;         // RTOS mutex for thread‑safe access
};
