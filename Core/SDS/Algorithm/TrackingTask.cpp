#include "TrackingTask.hpp"
#include "MyDebug.hpp"

void TrackingTask::process(const SRPFrame& frame, DistanceEstimator& dist)
{
    // DEBUG: Was kommt wirklich an?
    char dbg[128];
    snprintf(dbg, sizeof(dbg), "%p, %d, %3.0f, %.1f, %lu %d",frame.srp_grid, frame.ntheta, frame.dtheta_deg, frame.azimuth_deg, frame.timestamp_ms, frame.valid );
    MyDebug::instance().pGfx->text8x12(10, 100, dbg, Color::Yellow);

    if (!frame.valid)
        return;

    DistanceResult r = dist.process(frame);

    // DEBUG: Was liefert DistanceEstimator?
    snprintf(dbg, sizeof(dbg),
             "DistEst: valid=%d, dist=%.1f, conf=%.2f, az=%.1f",
             r.valid,
             r.distance_m,
             r.confidence,
             r.azimuth_deg);

    MyDebug::instance().pGfx->text8x12(10, 120, dbg, Color::Cyan);

    if (!r.valid)
        return;

    last_.azimuth_deg  = r.azimuth_deg;
    last_.distance_m   = r.distance_m;
    last_.confidence   = r.confidence;
    last_.timestamp_ms = r.timestamp_ms;
    last_.valid        = true;
}

TrackState TrackingTask::getLastState() const
{
    return last_;
}
