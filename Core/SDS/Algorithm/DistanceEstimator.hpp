/*
 * DistanceEstimator.hpp
 *
 * Physical Distance Estimation Model
 * ----------------------------------
 * This class implements a lightweight, real‑time distance estimator based
 * on the SRP‑PHAT peak amplitude. The underlying physical assumption is
 * the classical spherical spreading model:
 *
 *      A ∝ 1 / r
 *
 * where:
 *      A = SRP peak amplitude
 *      r = distance between source and microphone array
 *
 * The estimator uses:
 *
 *      r_est = k_ref / (A + eps)
 *
 * with:
 *   - k_ref : empirical scaling factor (depends on array gain, room, DSP)
 *   - eps   : stabilization term to avoid division by zero
 *
 * Confidence Model:
 * -----------------
 * Confidence is derived from the normalized peak amplitude:
 *
 *      confidence = A / (A + 1)
 *
 * yielding values in the range 0..1.
 *
 * Integration in SDS Pipeline:
 * ----------------------------
 * The estimator consumes SRPFrame objects produced by SRPPhat or
 * SRPPhatFast and outputs DistanceResult structures used by:
 *
 *   - SystemManager (tracking)
 *   - LCDTask (visualization)
 *   - SDS_Data (global state)
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once
#include <Model.hpp>
#include <cmath>

class DistanceEstimator
{
public:
    // Constructor: sets scaling factor and stabilization epsilon
    DistanceEstimator(float k_ref = 100.0f, float eps = 1e-3f)
        : k_ref_(k_ref), eps_(eps)
    {}

    // Process one SRP frame and compute distance + confidence
    DistanceResult process(const SRPFrame& frame)
    {
        DistanceResult result;

        // Validate frame
        if (!frame.valid)
        {
            result.distance_m = 0.0f;
            result.confidence = 0.0f;
            return result;
        }

        // Validate SRP grid
        if (frame.srp_grid == nullptr || frame.ntheta <= 0)
        {
            result.distance_m = 0.0f;
            result.confidence = 0.0f;
            return result;
        }

        // Validate peak amplitude
        float A = frame.peak_value;
        if (A <= 0.0f)
        {
            result.distance_m = 0.0f;
            result.confidence = 0.0f;
            return result;
        }

        // Physical 1/r model
        float dist = k_ref_ / (A + eps_);
        result.distance_m = dist;

        // Confidence: normalized amplitude
        float conf = A / (A + 1.0f);
        result.confidence = conf;

        return result;
    }

private:
    float k_ref_;  // scaling factor for 1/r model
    float eps_;    // stabilization term
};
