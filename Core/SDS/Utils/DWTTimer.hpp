/*
 * DWTTimer.hpp
 *
 * STM32F7 DWT Cycle Counter Timer
 * -------------------------------
 * This class provides a lightweight, high‑precision timing utility based on
 * the ARM Cortex‑M7 DWT (Data Watchpoint and Trace) cycle counter. The DWT
 * counter increments every CPU clock cycle and allows sub‑microsecond timing
 * without relying on HAL timers or SysTick.
 *
 * Hardware Basis:
 * ---------------
 * The STM32F7 runs at 216 MHz:
 *
 *      1 cycle = 1 / 216e6 seconds
 *              ≈ 4.6296 ns
 *
 * Therefore:
 *
 *      microseconds = cycles / 216
 *      milliseconds = cycles / 216000
 *
 * The timer is ideal for:
 *   - DSP profiling (FFT, SRP‑PHAT, filtering)
 *   - RTOS task timing
 *   - debugging performance bottlenecks
 *
 * SDS Integration:
 * ----------------
 * The timer writes its measured duration directly into SDS_Data::debugTime,
 * allowing LCDTask and SystemManager to visualize DSP timing.
 *
 * Initialization:
 * ---------------
 * initDWT() enables:
 *   - TRCENA (Trace unit)
 *   - CYCCNT (cycle counter)
 *
 * Created on: Jul 6, 2026
 * Author: Stefan (310004)
 */

#ifndef INC_SDS_SYSTEM_DWTTIMER_HPP_
#define INC_SDS_SYSTEM_DWTTIMER_HPP_

#include "Model.hpp"
#include "stm32f7xx_hal.h"

class DWTTimer {

public:
    // Singleton instance
    static DWTTimer& instance() { static DWTTimer inst; return inst; }

    // Capture start cycle count
    inline void getStartTime() { start = DWT->CYCCNT; }

    // Capture stop cycle count
    inline void getStopTime()  { stop  = DWT->CYCCNT; }

    // Write measured time directly into SDS_Data::debugTime
    inline void setDifferenceInDebugTime() {
        stop = DWT->CYCCNT;
        model->setDebugTime( float(stop - start) / 216.0f );
    }

    // Return time difference in microseconds
    inline float getTimeDifferenceUs() { return float(stop - start) / 216.0f; }

    // Return time difference in milliseconds
    inline float getTimeDifferenceMs() { return float(stop - start) / 216000.0f; }

private:
    // Constructor initializes DWT hardware
    DWTTimer() { initDWT(); }

    // Enable DWT cycle counter
    inline void initDWT() {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;        // enable trace unit
        *((volatile uint32_t*)0xE0001FB0) = 0xC5ACCE55;        // unlock DWT
        DWT->CYCCNT = 0;                                       // reset counter
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                   // enable counter

        model = &SDS_Data::instance();                         // link to global data model
    }

    uint32_t start = 0;   // cycle count at start
    uint32_t stop  = 0;   // cycle count at stop

    SDS_Data* model;      // pointer to global SDS data model
};

#endif /* INC_SDS_SYSTEM_DWTTIMER_HPP_ */
