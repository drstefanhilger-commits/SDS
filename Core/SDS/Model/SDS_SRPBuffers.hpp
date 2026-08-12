/*
 * SDS_SRPBuffers.hpp
 *
 * SRP‑PHAT Buffer Architecture
 * ----------------------------
 * This header defines all intermediate buffers required for the SRP‑PHAT
 * (Steered Response Power with Phase Transform) algorithm. These buffers
 * are used by SRPPhat, SRPPhatFast, and DSPTask to perform:
 *
 *   - FFT of microphone frames
 *   - PHAT weighting
 *   - inverse FFT (cross‑correlation)
 *   - pairwise correlation accumulation
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
 *   4. SRP accumulation:
 *        SRP(theta) = Σ corr_ij[n_delay(theta)]
 *
 * Buffer Overview:
 * ----------------
 * fftIn[]       : input frame for KISS‑FFT
 * fftOut[]      : FFT output
 * phatSpec[]    : PHAT‑weighted spectrum
 * corrTime[]    : time‑domain correlation (IFFT)
 *
 * corrPair[][][]:
 *     corrPair[i][j][n] stores the full correlation curve for each
 *     microphone pair (i,j). This allows SRPPhatFast to reuse correlations
 *     across multiple azimuth hypotheses.
 *
 * Additional Fast‑SRP Buffers:
 * ----------------------------
 * frame[]       : temporary input frame
 * Xr[][], Xi[][]:
 *     real/imag FFT components for each microphone, used by optimized
 *     SRPPhatFast implementations to avoid repeated FFT calls.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#pragma once
#include "SDS_Params.hpp"
#include "kiss_fft.h"

// ---------------------------------------------------------------------------
// SRP‑PHAT intermediate buffers
// ---------------------------------------------------------------------------
struct SRPBuffers
{
    // FFT input/output buffers
    kiss_fft_cpx fftIn[SDS_FFT_LEN];
    kiss_fft_cpx fftOut[SDS_FFT_LEN];

    // PHAT spectrum and inverse FFT (correlation)
    kiss_fft_cpx phatSpec[SDS_FFT_LEN];
    kiss_fft_cpx corrTime[SDS_FFT_LEN];

    // Full correlation curves for all microphone pairs
    // corrPair[i][j][n] = correlation between mic i and mic j at lag n
    float corrPair[SDS_NUM_MICS][SDS_NUM_MICS][SDS_FFT_LEN];

    // Additional buffers used by SRPPhatFast
    float frame[SDS_FFT_LEN];                         // input frame
    float Xr[SDS_NUM_MICS][SDS_FFT_LEN];              // FFT real part
    float Xi[SDS_NUM_MICS][SDS_FFT_LEN];              // FFT imaginary part
};

// Global instance (allocated once)
extern SRPBuffers g_srp;
