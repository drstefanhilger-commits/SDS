/*
 * TrackingTask.hpp
 *
 *  Created on: Jul 28, 2026
 *      Author: 310004
 */

#pragma once
#include "Model.hpp"
#include "DistanceEstimator.hpp"

class TrackingTask
{
public:
    void process(const SRPFrame& frame, DistanceEstimator& dist);
    TrackState getLastState() const;

private:
    TrackState last_{};
};
