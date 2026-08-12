/*
 * UnitTestSignals.hpp
 *
 * Synthetic Microphone Test Signals for SRP/DSP Validation
 * --------------------------------------------------------
 * This header provides deterministic, synthetic microphone signals used
 * for unit testing the SDS (Sensor‑DSP‑System). The signals emulate:
 *
 *   - far‑field broadband noise sources
 *   - plane‑wave impulse sources for azimuth validation
 *   - fractional sample delays based on microphone geometry
 *
 * These test frames allow validation of:
 *
 *   - SRP‑PHAT (frequency‑domain DoA)
 *   - SRP‑DAS (amplitude‑based DoA)
 *   - DistanceEstimator (1/r model)
 *   - microphone buffering and DSP timing
 *
 * Far‑Field Model:
 * ----------------
 * For a source at azimuth θ and distance r:
 *
 *      τ_ch = dot(micPos[ch], u(θ)) / c
 *
 * where:
 *      u(θ) = steering unit vector
 *      c    = speed of sound
 *
 * The microphone signal is generated as:
 *
 *      mic[ch][n] = src[n − τ_ch * fs]
 *
 * using fractional delay interpolation.
 *
 * Fractional Delay:
 * -----------------
 * sampleWithDelay() implements:
 *
 *      s[n − d] = (1 − frac) * s[floor(n − d)]
 *                 + frac * s[floor(n − d) + 1]
 *
 * ensuring smooth phase‑accurate delays for SRP testing.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once

#include <cmath>
#include <cstdint>
#include "Model.hpp"


class UnitTestSignals
{
public:
    // ---------------------------------------------------------------------
    // Far‑field broadband noise source
    //
    // angleDeg   : azimuth in degrees (0° = +x axis, CCW)
    // distance_m : 50 m ... 200 m (far‑field approximation)
    // noise      : noise factor (0 = no noise)
    //
    // Generates a full microphone frame with fractional delays.
    // ---------------------------------------------------------------------
    static void makeTestFrame360_farfield(float angleDeg,
                                          float distance_m,
                                          float noise,
                                          float mic[SDS_NUM_MICS][SDS_FRAME_LEN]);

    // ---------------------------------------------------------------------
    // Plane‑wave impulse signal for azimuth testing
    //
    // azDeg : azimuth in degrees
    //
    // Simulates an infinitely distant source (pure plane wave).
    // ---------------------------------------------------------------------
    static void generateTestSignalForAzimuth(float azDeg,
                                             float mic[SDS_NUM_MICS][SDS_FRAME_LEN]);

private:
    // ---------------------------------------------------------------------
    // Generate broadband noise source (shared across microphones)
    // ---------------------------------------------------------------------
    static void generateSourceNoise(float src[SDS_FRAME_LEN]);

    // ---------------------------------------------------------------------
    // Fractional delay using linear interpolation:
    //
    //      s[n − delaySamples]
    //
    // Used for far‑field and plane‑wave test signal generation.
    // ---------------------------------------------------------------------
    static float sampleWithDelay(const float* src,
                                 int length,
                                 float delaySamples,
                                 int n);
};
