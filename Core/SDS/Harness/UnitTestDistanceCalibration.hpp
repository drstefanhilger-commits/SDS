#pragma once

#include <vector>
#include "SDS_Params.hpp"

struct Result {
    float trueDist;
    float estDist;
};

class UnitTestDistanceCalibration {
public:
    static void runCalibration();

private:
    static float estimateDistance(const float mic[8][SDS_FRAME_LEN]);
    static void computeGainFit(const std::vector<Result>& data);
    static void computeLinearFit(const std::vector<Result>& data);
};
