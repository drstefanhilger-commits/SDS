/*
 * SRPPhat.cpp
 *
 * SRP‑PHAT Algorithm Implementation
 * ---------------------------------
 * This file implements the SRP‑PHAT (Steered Response Power with Phase
 * Transform) algorithm used for acoustic azimuth estimation in the SDS
 * (Sensor‑DSP‑System). The algorithm evaluates the summed GCC‑PHAT
 * correlation energy across all microphone pairs for a set of azimuth
 * hypotheses.
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
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#include "SRPPhat.hpp"


/* ============================================================
 *  Constructor: initialize geometry and FFT configurations
 * ============================================================ */

SRPPhat::SRPPhat()
{
    initGeometryFromSDS();

    // FFT configuration (two‑stage allocation required by KISS‑FFT)
    size_t memSize = 0;
    kiss_fft_alloc(SDS_FFT_LEN, 0, nullptr, &memSize);
    fftCfg = (kiss_fft_cfg)malloc(memSize);
    kiss_fft_alloc(SDS_FFT_LEN, 0, fftCfg, &memSize);

    size_t memSize2 = 0;
    kiss_fft_alloc(SDS_FFT_LEN, 1, nullptr, &memSize2);
    ifftCfg = (kiss_fft_cfg)malloc(memSize2);
    kiss_fft_alloc(SDS_FFT_LEN, 1, ifftCfg, &memSize2);

    configASSERT(fftCfg != nullptr);
    configASSERT(ifftCfg != nullptr);

    azMin     = SDS_AZ_MIN;
    azMax     = SDS_AZ_MAX;
    azStep    = SDS_AZ_STEP;
    currentAz = azMin;

    bestAz     = azMin;
    bestEnergy = -1e9f;
}

/* ============================================================
 *  Load microphone geometry from SDS parameters
 * ============================================================ */

void SRPPhat::initGeometryFromSDS()
{
    for (int i = 0; i < SDS_NUM_MICS; ++i)
    {
        micPos[i].x = SDS_MIC_POSITIONS[i][0];
        micPos[i].y = SDS_MIC_POSITIONS[i][1];
        micPos[i].z = SDS_MIC_POSITIONS[i][2];
    }
}

/* ============================================================
 *  Initialize azimuth scan
 * ============================================================ */

void SRPPhat::beginAzimuthScan(const float mics[SDS_NUM_MICS][SDS_FRAME_LEN],
                               float az_min,
                               float az_max,
                               float az_step)
{
    // Copy microphone frame data
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            micData[ch][n] = mics[ch][n];

    azMin     = az_min;
    azMax     = az_max;
    azStep    = az_step;
    currentAz = azMin;

    bestAz     = azMin;
    bestEnergy = -1e9f;

    prepareGCCPHAT();
}

/* ============================================================
 *  Evaluate one azimuth hypothesis
 * ============================================================ */

bool SRPPhat::stepAzimuthScan()
{
    if (currentAz > azMax)
        return true;

    const float az_rad = currentAz * (3.1415926535f / 180.0f);
    const float kx     = std::cos(az_rad);
    const float ky     = std::sin(az_rad);

    float tau[SDS_NUM_MICS];

    // Steering delays for each microphone
    for (int i = 0; i < SDS_NUM_MICS; ++i) {
        const float proj = micPos[i].x * kx + micPos[i].y * ky;
        tau[i] = proj * SoundSpeed_inf;
    }

    // Normalize delays relative to mic 0
    const float t0 = tau[0];
    for (int i = 0; i < SDS_NUM_MICS; ++i)
        tau[i] -= t0;

    float energySum = 0.0f;

    // Accumulate SRP energy across microphone pairs
    for (int i = 0; i < SDS_NUM_MICS; ++i) {
        for (int j = i + 1; j < SDS_NUM_MICS; ++j) {
            const float dt = (tau[i] - tau[j]) * SDS_SAMPLE_RATE;
            int delaySamples = static_cast<int>(dt);

            int idx = delaySamples;
            if (idx < 0)            idx += SDS_FFT_LEN;
            if (idx >= SDS_FFT_LEN) idx -= SDS_FFT_LEN;

            if (idx >= 0 && idx < SDS_FFT_LEN)
                energySum += g_srp.corrPair[i][j][idx];
        }
    }

    // Track best azimuth
    if (energySum > bestEnergy)
    {
        bestEnergy = energySum;
        bestAz     = currentAz;
    }

    currentAz += azStep;

    return (currentAz > azMax);
}

/* ============================================================
 *  Prepare GCC‑PHAT: FFT + PHAT weighting + IFFT
 * ============================================================ */

void SRPPhat::prepareGCCPHAT()
{
    // FFT per microphone channel
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch) {
        computeFFT(micData[ch], g_srp.Xr[ch], g_srp.Xi[ch]);
    }

    // GCC‑PHAT per microphone pair
    for (int i = 0; i < SDS_NUM_MICS; ++i) {
        for (int j = i + 1; j < SDS_NUM_MICS; ++j) {
            computePHATIFFT(g_srp.Xr[i], g_srp.Xi[i],
                            g_srp.Xr[j], g_srp.Xi[j],
                            g_srp.corrPair[i][j]);
        }
    }
}

/* ============================================================
 *  FFT computation using KISS‑FFT
 * ============================================================ */

void SRPPhat::computeFFT(const float* x, float* Xr, float* Xi)
{
    // Copy input samples
    for (int n = 0; n < SDS_FRAME_LEN; ++n) {
        g_srp.fftIn[n].r = x[n];
        g_srp.fftIn[n].i = 0.0f;
    }

    // Zero‑padding
    for (int n = SDS_FRAME_LEN; n < SDS_FFT_LEN; ++n) {
        g_srp.fftIn[n].r = 0.0f;
        g_srp.fftIn[n].i = 0.0f;
    }

    kiss_fft(fftCfg, g_srp.fftIn, g_srp.fftOut);

    for (int k = 0; k < SDS_FFT_LEN; ++k) {
        Xr[k] = g_srp.fftOut[k].r;
        Xi[k] = g_srp.fftOut[k].i;
    }
}

/* ============================================================
 *  GCC‑PHAT + IFFT
 * ============================================================ */

void SRPPhat::computePHATIFFT(const float* Xr, const float* Xi,
                              const float* Yr, const float* Yi,
                              float* outCorr)
{
    // PHAT weighting
    for (int k = 0; k < SDS_FFT_LEN; ++k) {
        float xr = Xr[k];
        float xi = Xi[k];
        float yr = Yr[k];
        float yi = Yi[k];

        float cr = xr * yr + xi * yi;
        float ci = xi * yr - xr * yi;

        float mag = std::sqrt(cr*cr + ci*ci);
        if (mag < 1e-6f) { mag = 1e-6f; }

        g_srp.phatSpec[k].r = cr / mag;
        g_srp.phatSpec[k].i = ci / mag;
    }

    // IFFT
    kiss_fft(ifftCfg, g_srp.phatSpec, g_srp.corrTime);

    // Normalize correlation
    for (int n = 0; n < SDS_FFT_LEN; ++n)
        outCorr[n] = g_srp.corrTime[n].r / float(SDS_FFT_LEN);
}

/* ============================================================
 *  Reset / Calibration / Filtering
 * ============================================================ */

void SRPPhat::resetScan()
{
    currentAz  = azMin;
    bestEnergy = -1.0f;
    bestAz     = 0.0f;
}

float SRPPhat::calibrateAzimuth(float rawAz)
{
    const float azOffset = 12.0f;
    const float azScale  = 0.98f;

    float az = (rawAz + azOffset) * azScale;

    if (az < 0.0f)    { az += 360.0f; }
    if (az >= 360.0f) { az -= 360.0f; }

    return az;
}

float SRPPhat::filterAzimuth(float newAz)
{
    static float filteredAz = 0.0f;
    const float alpha = 0.15f;

    float diff = newAz - filteredAz;

    if (diff > 180.0f)       { diff -= 360.0f; }
    else if (diff < -180.0f) { diff += 360.0f; }

    filteredAz += alpha * diff;

    if (filteredAz < 0.0f)    { filteredAz += 360.0f; }
    if (filteredAz >= 360.0f) { filteredAz -= 360.0f; }

    return filteredAz;
}
