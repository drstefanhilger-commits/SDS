/*
 * SRPPhat.hpp
 *
 * SRP‑PHAT Algorithm Description
 * ------------------------------
 * This class implements the SRP‑PHAT (Steered Response Power with Phase
 * Transform) algorithm used for acoustic azimuth estimation in the SDS
 * (Sensor‑DSP‑System). SRP‑PHAT computes the direction of arrival (DoA)
 * by evaluating the summed cross‑correlation energy across all microphone
 * pairs for a set of azimuth hypotheses.
 *
 * Mathematical Pipeline:
 * ----------------------
 * For each microphone pair (i,j):
 *
 *   1. FFT:
 *        X_i[k] = FFT(x_i[n])
 *        X_j[k] = FFT(x_j[n])
 *
 *   2. PHAT weighting:
 *        R_ij[k] = (X_i[k] * conj(X_j[k])) / |X_i[k] * conj(X_j[k])|
 *
 *   3. IFFT:
 *        corr_ij[n] = IFFT(R_ij[k])
 *
 * For each azimuth θ:
 *
 *   4. Steering delay:
 *        τ_ij(θ) = dot(micPos[i] - micPos[j], u(θ)) / c
 *
 *   5. SRP accumulation:
 *        SRP(θ) = Σ corr_ij[n_delay(τ_ij)]
 *
 * The azimuth with maximum SRP energy is selected as the DoA estimate.
 *
 * Geometry:
 * ---------
 * Microphone positions are loaded from SDS_Params.hpp and stored in micPos[].
 * The steering vector u(θ) is computed from the azimuth angle.
 *
 * Filtering & Calibration:
 * ------------------------
 * The raw azimuth estimate can be:
 *   - calibrated (calibrateAzimuth)
 *   - smoothed (filterAzimuth)
 *
 * These functions allow compensation for array geometry imperfections and
 * provide temporal stability.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once

#include "Model.hpp"
#include "kiss_fft.h"
#include <cmath>

class SRPPhat
{
public:
    // Constructor: initializes FFT configuration and geometry
    SRPPhat();

    // Begin azimuth scan over [az_min, az_max] with step az_step
    void beginAzimuthScan(const float mics[SDS_NUM_MICS][SDS_FRAME_LEN],
                          float az_min,
                          float az_max,
                          float az_step);

    // Perform one azimuth evaluation step
    bool stepAzimuthScan();

    // Prepare GCC‑PHAT (FFT + PHAT weighting)
    void prepareGCCPHAT();

    // Return best azimuth result
    float getResult() const { return bestAz; }

    // Reset scan state
    void resetScan();

    // Calibration and filtering
    float calibrateAzimuth(float rawAz);
    float filterAzimuth(float newAz);

    // Access SRP grid
    const float* getGrid() const { return srpGrid_; }
    int getNtheta() const { return ntheta_; }

    // Maximum SRP energy in grid
    float getMaxValue() const
    {
        float maxv = -1e30f;
        for (int i = 0; i < ntheta_; ++i)
        {
            if (srpGrid_[i] > maxv)
                maxv = srpGrid_[i];
        }
        return maxv;
    }

protected:
    // Microphone positions (loaded from SDS_Params)
    Vec3 micPos[SDS_NUM_MICS];

    // Scan parameters
    float azMin;
    float azMax;
    float azStep;

    float currentAz;
    float bestAz;
    float bestEnergy;

    // FFT configuration (KISS‑FFT)
    kiss_fft_cfg fftCfg;
    kiss_fft_cfg ifftCfg;

    // Audio data for each microphone
    float micData[SDS_NUM_MICS][SDS_FRAME_LEN];

    // Internal helpers
    void initGeometryFromSDS();
    void computeFFT(const float* x, float* Xr, float* Xi);
    void computePHATIFFT(const float* Xr, const float* Xi,
                         const float* Yr, const float* Yi,
                         float* outCorr);

private:
    // Inverse sound speed (1/c)
    static constexpr float SoundSpeed_inf = 1.0f / 343.0f;

    // SRP grid storage
    float srpGrid_[SDS_NUM_AZIMUTH_BINS];
    int   ntheta_;
};
