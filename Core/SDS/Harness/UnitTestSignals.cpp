/*
 * UnitTestSignals.cpp
 *
 * Synthetic Microphone Signals for SRP/DSP Validation
 * ---------------------------------------------------
 * This file implements deterministic synthetic microphone signals used
 * for validating the SDS (Sensor‑DSP‑System). The signals emulate:
 *
 *   - far‑field broadband noise sources
 *   - plane‑wave impulse sources for azimuth testing
 *   - fractional sample delays based on microphone geometry
 *
 * These signals allow controlled testing of:
 *
 *   - SRP‑PHAT (DoA estimation)
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
 * Impulse Model:
 * --------------
 * For azimuth testing, a plane‑wave impulse is generated. Each microphone
 * receives the impulse shifted by:
 *
 *      delaySamples = round( proj * fs / c )
 *
 * ensuring phase‑accurate DoA validation.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#include "UnitTestSignals.hpp"
#include <cstdlib>

// ---------------------------------------------------------
// Generate broadband noise source (shared across microphones)
// ---------------------------------------------------------
void UnitTestSignals::generateSourceNoise(float src[SDS_FRAME_LEN])
{
    for (int n = 0; n < SDS_FRAME_LEN; ++n)
    {
        float r = std::rand() / float(RAND_MAX);
        src[n] = r - 0.5f;   // white noise, mean ~0
    }
}

// ---------------------------------------------------------
// Fractional delay using linear interpolation
// ---------------------------------------------------------
float UnitTestSignals::sampleWithDelay(const float* src,
                                       int length,
                                       float delaySamples,
                                       int n)
{
    // We want s[n - delaySamples]
    float t = float(n) - delaySamples;
    int i0 = int(t);
    float frac = t - float(i0);

    if (i0 < 0 || i0 + 1 >= length)
        return 0.0f;

    float s0 = src[i0];
    float s1 = src[i0 + 1];

    return (1.0f - frac) * s0 + frac * s1;
}

// ---------------------------------------------------------
// Far‑field noise signal (azimuth + distance)
// ---------------------------------------------------------
//
// Conventions:
//  - 0° = +x axis (Mic 0)
//  - 90° = +y axis (Mic 2)
//  - CCW rotation
//  - Far‑field: plane wave, delay from direction only,
//    amplitude from distance (1/r)
// ---------------------------------------------------------
void UnitTestSignals::makeTestFrame360_farfield(float angleDeg,
                                                float distance_m,
                                                float noise,
                                                float mic[SDS_NUM_MICS][SDS_FRAME_LEN])
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;

    // Steering unit vector
    float theta = angleDeg * (float(M_PI) / 180.0f);
    float dx = std::cos(theta);
    float dy = std::sin(theta);

    // Far‑field amplitude ∝ 1/r
    const float A = 1.0f / distance_m;

    // Clear frame
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            mic[ch][n] = 0.0f;

    // Shared noise source (coherent)
    float src[SDS_FRAME_LEN];
    generateSourceNoise(src);

    // Per microphone: delay + amplitude + background noise
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        // Far‑field delay: projection onto direction vector
        float proj = x * dx + y * dy;
        float tau  = proj / c;          // seconds
        float delaySamples = tau * fs;  // fractional samples

        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            // Fractional delay sample
            float s = sampleWithDelay(src, SDS_FRAME_LEN, delaySamples, n);
            float v = A * s;

            // Add incoherent background noise
            if (noise > 0.0f)
            {
                float r = std::rand() / float(RAND_MAX);
                float n0 = (r - 0.5f) * noise * A;
                v += n0;
            }

            mic[ch][n] = v;
        }
    }
}

// ---------------------------------------------------------
// Impulse signal for azimuth testing (plane wave)
// ---------------------------------------------------------
//
// Conventions identical:
//  - 0° = +x, 90° = +y, CCW
//  - Reference impulse at Mic 0 near impulsePos
// ---------------------------------------------------------
void UnitTestSignals::generateTestSignalForAzimuth(float azDeg,
                                                   float mic[SDS_NUM_MICS][SDS_FRAME_LEN])
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;
    const float inv_c_fs = fs / c;

    // Steering unit vector
    float azRad = azDeg * (3.1415926535f / 180.0f);
    float dx = std::cos(azRad);
    float dy = std::sin(azRad);

    // Impulse position in reference microphone
    const int impulsePos = 40;

    // Clear all channels
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int i = 0; i < SDS_FRAME_LEN; ++i)
            mic[ch][i] = 0.0f;

    // Per microphone: integer delay approximation
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        float proj = x * dx + y * dy;
        float delaySamples_f = proj * inv_c_fs;

        int delaySamples = int(std::round(delaySamples_f));

        int pos = impulsePos + delaySamples;
        if (pos >= 0 && pos < SDS_FRAME_LEN)
            mic[ch][pos] = 1.0f;
    }
}
