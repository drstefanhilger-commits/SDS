/*
 * LCDTask.cpp
 *
 * Mathematical / Physical Description
 * -----------------------------------
 * This task renders real‑time DSP results (azimuth, distance, confidence,
 * timing metrics, etc.) onto the LCD using the LCDDriver and PixelEngine.
 *
 * The visualization is based on polar geometry:
 *
 *   Azimuth θ (degrees) is converted to radians:
 *
 *       θ_rad = θ * (π / 180)
 *
 *   Distance d (meters) is mapped to a radial pixel coordinate:
 *
 *       r_px = distFac * d
 *
 * The source position is drawn using:
 *
 *       x = x0 + r_px * cos(θ_rad)
 *       y = y0 + r_px * sin(θ_rad)
 *
 * where (x0, y0) is the center of the polar plot.
 *
 * The task also visualizes:
 *   - error bounds (errorAz, errorDist)
 *   - true vs. estimated values
 *   - frame counter
 *   - diagnostic timing values
 *
 * Timing Model:
 * -------------
 * The task runs periodically with sampling period:
 *
 *      T = delayMs / 1000 seconds
 *
 * ensuring deterministic frame updates and stable visualization.
 *
 * Created on: Aug 10, 2026
 * Author: Stefan (310004)
 */

#include "LCDTask.hpp"

// ---------------------------------------------------------------------------
// Constructor: initializes base task and LCD graphics engine.
// ---------------------------------------------------------------------------
LCDTask::LCDTask()
    : TaskBase(2024, 50, osPriorityNormal)
{
    // Initialize LCD graphics engine with framebuffer and resolution.
    pGfx->init(pGfx->pStartFrameBuffer, 480, 272);
}

// ---------------------------------------------------------------------------
// Initialization hook — executed once before periodic processing.
// ---------------------------------------------------------------------------
void LCDTask::onStart()
{
    // LCD initialization or splash screen could be placed here.
}

// ---------------------------------------------------------------------------
// Periodic update — renders DSP results to the LCD.
// ---------------------------------------------------------------------------
void LCDTask::runOnce()
{
    char buf[128];

    // Clear screen
    pGfx->clear(Color::Black);

    // Version
    pGfx->text8x12(420, 10, "1.00", Color::White);

    // Draw reference geometry
    pGfx->line(220, 10, 220, 262, Color::Red);     // vertical reference line
    pGfx->circle(x0, y0, R,   Color::White);       // outer circle
    pGfx->circle(x0, y0, dm.getDebugValue3()*distFac, Color::Yellow);       // inner circle (trueDist)

    // Convert DSP azimuth to radians
    float angle = dm.getAzimuth() * deg2rad;

    // Convert DSP distance to pixel radius
    float r1 = dm.getDistance() * distFac;

    // Compute source position in display coordinates
    int x1 = x0 + static_cast<int>(r1 * cosf(angle));
    int y1 = y0 + static_cast<int>(r1 * sinf(angle));

    // Draw line from center to estimated source position
    pGfx->line(x0, y0, x1, y1, Color::Red);
    pGfx->circle(x1, y1, 3, Color::Green);
    // -----------------------------------------------------------------------
    // Azimuth visualization and error metrics
    // -----------------------------------------------------------------------
    difAz = fabs(dm.getAzimuth() - dm.getDebugValue2());
    Color color = (difAz < errorAz ? Color::Green : Color::Red);

    snprintf(buf, sizeof(buf), "Azimuth        %.3f", dm.getAzimuth());
    pGfx->text8x12(10, 10, buf, color);

    snprintf(buf, sizeof(buf), "Dif Azimuth    %.3f", difAz);
    pGfx->text8x12(10, 20, buf, color);

    // -----------------------------------------------------------------------
    // Distance visualization and error metrics
    // -----------------------------------------------------------------------
    float difDist = fabsf(dm.getDistance() - dm.getDebugValue3());
    percent = 100.0f * difDist / dm.getDebugValue3();
    color = (percent < errorDist ? Color::Green : Color::Red);

    snprintf(buf, sizeof(buf), "Distance       %.1f", dm.getDistance());
    pGfx->text8x12(10, 30, buf, color);

    snprintf(buf, sizeof(buf), "Dif Distance   %.3f", difDist);
    pGfx->text8x12(10, 40, buf, color);

    // -----------------------------------------------------------------------
    // True values (from MicTask simulation)
    // -----------------------------------------------------------------------
    snprintf(buf, sizeof(buf), "True Azimuth   %.3f", dm.getDebugValue2());
    pGfx->text8x12(10, 160, buf, Color::White);

    snprintf(buf, sizeof(buf), "True Distance  %.3f", dm.getDebugValue3());
    pGfx->text8x12(10, 170, buf, Color::White);

    // -----------------------------------------------------------------------
    // Debug values (temporary placeholders)
    // -----------------------------------------------------------------------
    snprintf(buf, sizeof(buf), "Loop Mic       %.3f", static_cast<float>(x1));
    pGfx->text8x12(10, 180, buf, Color::White);

    snprintf(buf, sizeof(buf), "SRPPhat Time   %.3f", static_cast<float>(y1));
    pGfx->text8x12(10, 190, buf, Color::White);

    snprintf(buf, sizeof(buf), "Frame: %d", frame_nr++);
    pGfx->text8x12(10, 200, buf, Color::White);

    // Swap framebuffer (double buffering)
    pGfx->activateFrameBuffer();
}

// ---------------------------------------------------------------------------
// Internal rendering logic (currently unused).
// ---------------------------------------------------------------------------
void LCDTask::process()
{
    // Placeholder for extended rendering logic.
}
