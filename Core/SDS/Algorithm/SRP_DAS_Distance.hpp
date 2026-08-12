/*
 * SRP_DAS_Distance.hpp
 *
 * Delay‑and‑Sum Amplitude Model (DAS)
 * -----------------------------------
 * This class implements a lightweight amplitude‑based SRP method using
 * classical Delay‑and‑Sum beamforming. Unlike SRP‑PHAT, which uses
 * GCC‑PHAT correlation in the frequency domain, this approach evaluates
 * the summed absolute amplitude after steering each microphone signal
 * according to a hypothesized azimuth angle.
 *
 * Mathematical Model:
 * -------------------
 * For each microphone channel ch:
 *
 *      τ_ch(θ) = dot(micPos[ch], u(θ)) / c
 *
 * where:
 *      u(θ) = steering unit vector for azimuth θ
 *      c    = speed of sound
 *
 * The signal is shifted by:
 *
 *      delaySamples = τ_ch * fs
 *
 * and the amplitude is accumulated:
 *
 *      A(θ) = Σ_ch Σ_n | mic[ch][n + delaySamples] |
 *
 * The azimuth with maximum amplitude A(θ) is selected as the DoA estimate.
 *
 * SRPFrame Integration:
 * ---------------------
 * The result is packaged into an SRPFrame structure, making it compatible
 * with the DistanceEstimator and the rest of the SDS pipeline:
 *
 *      - srp_grid[]     : amplitude values for each azimuth
 *      - peak_value     : maximum amplitude
 *      - azimuth_deg    : angle of maximum amplitude
 *      - ntheta         : number of azimuth bins
 *      - valid          : frame validity
 *
 * This provides a simple, amplitude‑sensitive alternative to SRP‑PHAT.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once

#include <cmath>
#include "Model.hpp"

// ======================================================
//   SRP‑DAS‑Distance (Amplitude‑based SRP)
// ======================================================
class SRP_DAS_Distance
{
public:
    SRP_DAS_Distance() {}

    // ------------------------------------------------------
    //  Compute amplitude for one azimuth using Delay‑and‑Sum
    // ------------------------------------------------------
    float computeAmplitude(const float mic[SDS_NUM_MICS][SDS_FRAME_LEN],
                           float angleDeg)
    {
        const float c  = 343.0f;
        const float fs = SDS_SAMPLE_RATE;

        float theta = angleDeg * (3.1415926535f / 180.0f);
        float dx = std::cos(theta);
        float dy = std::sin(theta);

        float sumAmp = 0.0f;

        for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        {
            float x = SDS_MIC_POSITIONS[ch][0];
            float y = SDS_MIC_POSITIONS[ch][1];

            // Steering delay
            float tau = (x * dx + y * dy) / c;
            int delaySamples = static_cast<int>(tau * fs);

            // Accumulate amplitude
            for (int n = 0; n < SDS_FRAME_LEN; ++n)
            {
                int idx = n + delaySamples;
                if (idx >= 0 && idx < SDS_FRAME_LEN)
                {
                    sumAmp += std::fabs(mic[ch][idx]);
                }
            }
        }

        return sumAmp;
    }

    // ------------------------------------------------------
    //  Full DAS scan + SRPFrame generation
    // ------------------------------------------------------
    SRPFrame makeFrame(const float mic[SDS_NUM_MICS][SDS_FRAME_LEN],
                       float azMinDeg,
                       float azMaxDeg,
                       float azStepDeg,
                       uint32_t timestamp_ms)
    {
        static float srpGrid[SDS_NUM_AZIMUTH_BINS];

        int   ntheta   = 0;
        float bestAz   = azMinDeg;
        float bestPeak = 0.0f;

        // Full azimuth scan
        for (float az = azMinDeg; az <= azMaxDeg; az += azStepDeg)
        {
            float val = computeAmplitude(mic, az);
            srpGrid[ntheta] = val;

            if (val > bestPeak)
            {
                bestPeak = val;
                bestAz   = az;
            }

            ++ntheta;
            if (ntheta >= SDS_NUM_AZIMUTH_BINS)
                break;
        }

        // Build SRPFrame
        SRPFrame frame{};
        frame.valid        = (bestPeak > 0.0f);
        frame.srp_grid     = srpGrid;
        frame.ntheta       = ntheta;
        frame.dtheta_deg   = azStepDeg;
        frame.azimuth_deg  = bestAz;
        frame.peak_value   = bestPeak;
        frame.timestamp_ms = timestamp_ms;

        return frame;
    }
};
