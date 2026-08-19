/*
 * MicTask.hpp
 *
 * Mathematical / Physical Description
 * -----------------------------------
 * This task simulates microphone array input signals for testing the
 * SRP‑PHAT and distance‑estimation pipeline. Instead of acquiring real
 * acoustic data from the ADAU7118 + DMA subsystem, the MicTask generates
 * synthetic pressure signals p_i(t) for each microphone channel.
 *
 * Physical Model:
 * ---------------
 * A simulated acoustic source is defined by:
 *
 *      - trueAz   : true azimuth angle (degrees)
 *      - trueDist : true distance (meters)
 *
 * For each microphone i, the simulated pressure signal is:
 *
 *      p_i(t) = A * sin( 2π f (t - τ_i) )
 *
 * where:
 *   - A is the amplitude
 *   - f is the test frequency
 *   - τ_i is the propagation delay based on geometry:
 *
 *      τ_i = (d_i / c)
 *
 * with:
 *   - d_i = geometric distance from source to microphone i
 *   - c   = speed of sound (≈ 343 m/s)
 *
 * The parameters:
 *   d_az   : azimuth increment per iteration
 *   d_Dist : distance increment per iteration
 *
 * allow controlled motion of the virtual source, enabling:
 *   - SRP‑PHAT validation
 *   - distance estimator calibration
 *   - end‑to‑end DSP pipeline testing
 *
 * Timing Model:
 * -------------
 * The task runs periodically with sampling period:
 *
 *      T = delayMs / 1000 seconds
 *
 * ensuring deterministic synthetic signal generation.
 *
 * Created on: Aug 10, 2026
 * Author: Stefan (310004)
 */

#pragma once
#include "TaskBase.hpp"
#include "Model.hpp"

#include "UnitTestSignals.hpp"

class MicTask : public TaskBase
{
public:
    // Singleton instance — ensures only one microphone simulator runs.
    static MicTask& instance() { static MicTask inst; return inst; }

protected:
    // Periodic DSP update — performs SRP‑PHAT and distance estimation.
    void runOnce() override;

    // Initialization hook — prepares buffers, timers, and DSP structures.
    void onStart() override;

private:
    // Constructor: initializes base task and simulation parameters.
    MicTask();

    // Handler for sampling
    void detectHandler();

    // Handler for calibrating
    void claibrateHandler();

    // Error-Handler
    void errorHandler();

    // Read Microphone
    void readMic();

    // Simulate Microphone
    void simulateMic();

    // Mic-Error
    void errorMic();

private:
    // Global data model for publishing simulated microphone frames.
    SDS_Data& dm = SDS_Data::instance();

    // Microphone buffer manager (triple‑buffered DMA simulation).
    SDS_MicrophoneBuffer& micBufferManager = SDS_MicrophoneBuffer::instance();

    // Simulation parameters for virtual source motion.
    float d_az     = 0.0543f;   // azimuth increment per iteration
    float trueAz   = 0.0f;      // current true azimuth
    float d_Dist   = 4.32f;     // distance increment per iteration
    float trueDist = 50.0f;     // current true distance
};
