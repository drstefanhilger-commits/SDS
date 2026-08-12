#pragma once

#include <SDS_Structs.hpp>
#include <SRPPhat.hpp>

#include "SRPPhat.hpp"
#include "SRP_DAS_Distance.hpp"
#include "DistanceEstimator.hpp"
#include "SDS_Params.hpp"

#include "UnitTestSignals.hpp"
#include "UnitTestDistanceCalibration.hpp"
#include "TrackingTask.hpp"
#include "MyDebug.hpp"


class UnitTest
{
public:

	UnitTest() {};

	void run_calibration();
	void run_srp_dynamic_unittest();
	void run_srp_dynamic_unittest_with_distance();
	void run_srp_dynamic_unittest_for_DistanceEstimator();
	void run_srp_dynamic_unittest_for_Signals();

	bool run_srp_old_unittest();
    bool run_srp_unittest_1();
    bool run_srp_unittest();
    bool run_srp_fast_unittest();
    bool run_distance_unittest();

	void generateTestSignalForAzimuth(float azDeg);
	void generateTestSignalForAzimuthAndDistance(float azDeg, float distanceMeters);
	void generateTestFrame(float angleDeg, float distance_m, float noise);
	void makeTestFrame360(float angleDeg, float distance_m, float noise);
	void makeTestFrame360_farfield(float angleDeg, float distance_m, float noise);

private:
    float mic[SDS_NUM_MICS][SDS_FRAME_LEN];
};
