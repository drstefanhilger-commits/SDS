/*
 * SDS_Structs.hpp
 *
 * SDS Data Model Description
 * --------------------------
 * This header defines all shared enums and data structures used across the
 * SDS (Sensor‑DSP‑System). These structures form the backbone of the
 * real‑time DSP pipeline, connecting:
 *
 *   - MicTask (acquisition / simulation)
 *   - SRPTask (localization)
 *   - DSPTask (FFT / filtering)
 *   - LCDTask (visualization)
 *   - SystemManager (state aggregation)
 *
 * The structs are designed for deterministic real‑time behavior and contain:
 *
 *   - SRP‑PHAT frames
 *   - distance estimation results
 *   - tracking states
 *   - system states
 *   - event messages for RTOS queues
 *
 * All timestamps are expressed in milliseconds and are intended to be
 * generated using DWTTimer or osKernelGetTickCount().
 *
 * Created on: Jul 28, 2026
 * Author: Stefan (310004)
 */

#pragma once
#include <cstdint>
#include <vector>

// ======================================================
//  SDS DATA MODEL – Shared Event Types
// ======================================================
//
// These event types are used in SDS_DataEvent and RTOS message queues.
// They allow tasks to signal updates without polling.
//
enum class SDS_DataEventType : uint8_t {
    SRP_UPDATE,      // SRP‑PHAT result updated
    DEBUG_UPDATE,    // debug values updated
    MICBLOCK_UPDATE  // new microphone block available
};

// ======================================================
//  SDS DATA MODEL – Mode Event Types
// ======================================================
//
// These event types are used in PC-Monitor message.
// They allow tasks to change the mode.
//
enum class SDS_ModeEventType : uint32_t {
    DETECT    = 1,      	// Standard Detecting
    CALIBRATE = 2,    	    // Calibrating
    READ      = 3, 			// Reading and Writing via USB
	ERROR	  = 99
};

// ======================================================
//  SDS DATA MODEL – Shared Structures
// ======================================================

// -------------------- 3D Vector (microphone positions) --------------------
struct Vec3
{
    float x;
    float y;
    float z;
};

// -------------------- SDS Event (high‑level DSP result) --------------------
struct SDSEvent {
    float    azimuth_deg;   // estimated azimuth (degrees)
    float    distance_m;    // estimated distance (meters)
    float    confidence;    // confidence 0..1
    uint32_t timestamp_ms;  // event timestamp
};

// -------------------- SRP FRAME (full SRP‑PHAT result) --------------------
struct SRPFrame {
    const float* srp_grid;     // pointer to SRP‑PHAT grid (size ntheta)
    int          ntheta;       // number of azimuth bins
    float        dtheta_deg;   // angular resolution (degrees)
    float        azimuth_deg;  // peak azimuth (maximum SRP energy)
    uint32_t     timestamp_ms; // timestamp of SRP computation
    bool         valid;        // frame validity
    float        peak_value;   // maximum SRP amplitude
};

// -------------------- Distance Estimation Result --------------------
struct DistanceResult {
    float    distance_m;       // estimated distance
    float    confidence;       // confidence 0..1
    float    azimuth_deg;      // azimuth used for tracking
    uint32_t timestamp_ms;     // timestamp
    bool     valid;            // result validity
};

// -------------------- Tracking State (filtered DSP output) --------------------
struct TrackState {
    float    azimuth_deg;      // filtered azimuth
    float    distance_m;       // filtered distance
    float    confidence;       // filtered confidence
    uint32_t timestamp_ms;     // timestamp
    bool     valid;            // state validity
};

// -------------------- System State (aggregated UI state) --------------------
struct SystemState {
    float    angle_deg;        // current angle
    float    distance_m;       // current distance
    float    confidence;       // confidence
    bool     detected;         // detection flag
    uint8_t  activeMask;       // active microphone mask
    uint8_t  activeCount;      // number of active microphones
};

// -------------------- Detection State --------------------
struct DetectionState {
    bool  detected;            // detection flag
    float confidence;          // confidence 0..1
};

// -------------------- SDS Data Event (RTOS queue message) --------------------
struct SDS_DataEvent {
    SDS_DataEventType type;    // event type
    float              processTime; // execution time in ms
};

// -------------------- SDS message --------------------
struct SDS_UnixTimeSync {
	uint8_t id			= 0x01;
	uint8_t size[3]		= {0x00, 0x00, 0x0C};
	uint8_t time[4]		= {0x00, 0x00, 0x00, 0x00};
	uint8_t crc[4]      = {0x00, 0x00, 0x00, 0x00};
};

// -------------------- SDS message --------------------
struct SDS_ModeChange {
	uint8_t id 			= 0x02;
	uint8_t size[3]		= {0x00, 0x00, 0x0C};
	uint8_t mode[4]		= {0x00, 0x00, 0x00, 0x01};
	uint8_t crc[4]		= {0x00, 0x00, 0x00, 0x00};
};

