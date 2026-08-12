/*
 * SDS_Params.hpp
 *
 * Global Parameter Description
 * ----------------------------
 * This header defines all global constants used throughout the SDS
 * (Sensor‑DSP‑System). These parameters configure:
 *
 *   - microphone array geometry
 *   - DSP frame sizes and FFT lengths
 *   - SRP‑PHAT azimuth grid resolution
 *   - simulation parameters for WSL/STM32 synchronization
 *   - UI update rates
 *   - bandpass filter limits
 *
 * The parameters are shared between:
 *   - MicTask (acquisition / simulation)
 *   - SRPTask (localization)
 *   - DSPTask (FFT / filtering)
 *   - LCDTask (visualization)
 *
 * All values are deterministic and compile‑time constant to ensure
 * reproducible behavior across STM32F7 and WSL2 simulation.
 *
 * Microphone Geometry:
 * --------------------
 * The array is an 8‑microphone circular ring with radius SDS_MIC_RADIUS.
 * Positions are defined in Cartesian coordinates (x,y,z) and match the
 * WSL simulation environment exactly.
 *
 * SRP‑PHAT Grid:
 * --------------
 * Azimuth scanning is performed from SDS_AZ_MIN to SDS_AZ_MAX in steps
 * of SDS_AZ_STEP. The number of bins is:
 *
 *      SDS_NUM_AZIMUTH_BINS = 361
 *
 * for a full 360° scan with 1° resolution.
 *
 * Created on: Aug 12, 2026
 * Author: Stefan (310004)
 */

#ifndef SDS_PARAMS_HPP
#define SDS_PARAMS_HPP

// Enable STM32F7 runtime and WSL test data
#define SDS_STM32_ACTIVE
#define SDS_TEST_DATA

// ======================================================
//  Microphone Array Geometry
// ======================================================

// Number of microphones in the circular array
static constexpr int SDS_NUM_MICS = 8;

// Radius of microphone ring (meters)
static constexpr float SDS_MIC_RADIUS = 0.20f;

// Exact microphone positions (WSL‑synchronized)
static constexpr float SDS_MIC_POSITIONS[SDS_NUM_MICS][3] = {
    {  SDS_MIC_RADIUS * 1.0f,        0.0f,                          0.0f },  // 0°
    {  SDS_MIC_RADIUS * 0.70710678f, SDS_MIC_RADIUS * 0.70710678f,  0.0f },  // 45°
    {  0.0f,                         SDS_MIC_RADIUS * 1.0f,         0.0f },  // 90°
    { -SDS_MIC_RADIUS * 0.70710678f, SDS_MIC_RADIUS * 0.70710678f,  0.0f },  // 135°
    { -SDS_MIC_RADIUS * 1.0f,        0.0f,                          0.0f },  // 180°
    { -SDS_MIC_RADIUS * 0.70710678f, -SDS_MIC_RADIUS * 0.70710678f, 0.0f },  // 225°
    {  0.0f,                        -SDS_MIC_RADIUS * 1.0f,         0.0f },  // 270°
    {  SDS_MIC_RADIUS * 0.70710678f, -SDS_MIC_RADIUS * 0.70710678f, 0.0f }   // 315°
};

// ======================================================
//  DSP Core Parameters
// ======================================================

// Sample rate (Hz)
static constexpr int SDS_SAMPLE_RATE = 48000;

// FFT length
#define SDS_FFT_LEN 256

// Frame length (half FFT)
#define SDS_FRAME_LEN (SDS_FFT_LEN / 2)

// DMA/I2S block size for STM32F7
static constexpr int SDS_BLOCK_SIZE = 128;

// Number of power bins for UI visualization
static constexpr int SDS_NUM_POWER_BINS = 36;

// ======================================================
//  SRP‑PHAT Azimuth Grid
// ======================================================
static constexpr float SDS_AZ_MIN  = 0.0f;
static constexpr float SDS_AZ_MAX  = 360.0f;
static constexpr float SDS_AZ_STEP = 1.0f;

// Number of azimuth bins (full 360° scan)
#define SDS_NUM_AZIMUTH_BINS 361

// ======================================================
//  Simulation Parameters (WSL / STM32 Synchronization)
// ======================================================
static constexpr float SDS_SIM_INIT_DISTANCE = 100.0f;
static constexpr float SDS_SIM_MIN_DISTANCE  = 10.0f;
static constexpr float SDS_SIM_DISTANCE_STEP = 1.0f;
static constexpr float SDS_SIM_ANGLE_STEP    = 1.01f;
static constexpr int   SDS_SIM_NUM_SAMPLES   = 2048;

// ======================================================
//  UI / Display Parameters
// ======================================================
static constexpr int SDS_UI_FPS  = 60;
static constexpr int SDS_DSP_FPS = 33;

// ======================================================
//  RadarWidget Visualization
// ======================================================
static constexpr float SDS_RADAR_MAX_POWER = 100.0f;

// ======================================================
//  Distance Estimation Gain
// ======================================================
static constexpr float SDS_DIST_GAIN = 20000.0f;

// ======================================================
//  Bandpass Filter (800–3000 Hz)
// ======================================================
static constexpr float SDS_BANDPASS_LOW  = 800.0f;
static constexpr float SDS_BANDPASS_HIGH = 3000.0f;

// ======================================================
//  Versioning
// ======================================================
static constexpr int SDS_PARAM_VERSION = 3;

#endif
