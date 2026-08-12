#include "UnitTest.hpp"
#include "az170.hpp"      // csvData[8][512]
#include <cmath>
#include <stdio.h>
#include <cstdlib>


void UnitTest::run_calibration() {
	UnitTestDistanceCalibration::runCalibration();
}

void UnitTest::run_srp_dynamic_unittest_for_Signals() {
    struct TestCase {
        float az;
        float dist;
    };

    const TestCase tests[] = {
        {  0,  50 },
        { 30,  80 },
        { 45, 120 },
        { 60, 200 },
        { 90, 300 },
        {120, 150 },
        {277,  60 },
        {330,  40 },
        {345,  30 },
        { 77, 100 },
        { 91, 200 },
        {222, 300 }
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    char buf[128];
    int y = 20;

    SRP_DAS_Distance das;
    float fac = SDS_DIST_GAIN/91.025f*0.85f;
    DistanceEstimator distEst(fac, 1e-3f);

    for (int t = 0; t < numTests; ++t)
    {
        float trueAz   = tests[t].az;
        float trueDist = tests[t].dist;
        // 1) Testsignal erzeugen
        UnitTestSignals::makeTestFrame360_farfield(trueAz, trueDist, 0.01f, mic);

        // 2) SRPPhat + Energy
        SRPPhat srp;
        MyDebug::instance().start();
        srp.beginAzimuthScan(mic, 0.0f, 360.0f, 1.0f);
        while(!srp.stepAzimuthScan()) {}
        float bestAz = srp.getResult();

        // 3) DistanceEstimator
        SRPFrame frame = das.makeFrame(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP, 1000);
        DistanceResult dr = distEst.process(frame);
        MyDebug::instance().stop();

        float azDiff = fabsf(bestAz - trueAz);
        float bestDist = dr.distance_m;
//        float bestConf = dr.confidence;
        float distDiff = fabsf(bestDist - trueDist);
        float percent = 100.0f* distDiff/trueDist;

        // 3) Ausgabe
		snprintf(buf, sizeof(buf),
				 "Az=%3.0f dA=%4.1f d=%4.1f dd=%3.1f p=%3.1f dt=%4.3f %s",
				 bestAz, azDiff, bestDist, distDiff, percent,
				 MyDebug::instance().diffMs(),
				 (azDiff < 2.0f ? "OK" : "FAIL"));

        MyDebug::instance().pGfx->text8x12(10, y, buf, Color::White);
//            (azDiff < 2.0f && distDiff < 50.0f ? Color::Green : Color::Red)
        y += 12;

    }
}

void UnitTest::run_srp_dynamic_unittest_for_DistanceEstimator() {
    struct TestCase {
        float az;
        float dist;
    };

    const TestCase tests[] = {
        {  0,  50 },
        { 30,  80 },
        { 45, 120 },
        { 60, 200 },
        { 90, 300 },
        {120, 150 },
        {277,  60 },
        {330,  40 },
        {345,  30 },
        { 77, 100 },
        { 91, 200 },
        {222, 300 }
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    char buf[128];
    int y = 20;

    SRP_DAS_Distance das;
    DistanceEstimator distEst(SDS_DIST_GAIN/91.025f, 1e-3f);

    for (int t = 0; t < numTests; ++t)
    {
        float trueAz   = tests[t].az;
        float trueDist = tests[t].dist;

        // 1) Testsignal erzeugen
//        generateTestSignalForAzimuth(trueAz);
//        makeTestFrame360(trueAz, trueDist, 0.01f);
        makeTestFrame360_farfield(trueAz, trueDist, 0.01f);

        // 2) SRPPhat + Energy
        SRPPhat srp;
        MyDebug::instance().start();
        srp.beginAzimuthScan(mic, 0.0f, 360.0f, 1.0f);
        while(!srp.stepAzimuthScan()) {}
        float bestAz = srp.getResult();

        // 3) DistanceEstimator
        SRPFrame frame = das.makeFrame(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP, 1000);
        DistanceResult dr = distEst.process(frame);

        float azDiff = fabsf(bestAz - trueAz);
        MyDebug::instance().stop();
        float bestDist = dr.distance_m;
//        float bestConf = dr.confidence;
        float distDiff = fabsf(bestDist - trueDist);
        float percent = 100.0f* distDiff/trueDist;

        // 3) Ausgabe
        snprintf(buf, sizeof(buf),
            "Az=%3.0f dA=%4.1f d=%3.1f dd=%2.1f p=%2.1f dt=%.3f %s",
            bestAz, azDiff, bestDist, distDiff, percent, MyDebug::instance().diffMs(), (azDiff < 2.0f ? "OK" : "FAIL"));

        MyDebug::instance().pGfx->text8x12(10, y, buf, Color::White);
//            (azDiff < 2.0f && distDiff < 50.0f ? Color::Green : Color::Red)
        y += 12;
    }
}

void UnitTest::run_srp_dynamic_unittest_with_distance()
{
    struct TestCase {
        float az;
        float dist;
    };

    const TestCase tests[] = {
        {  0,  50 },
        { 30,  80 },
        { 45, 120 },
        { 60, 200 },
        { 90, 300 },
        {120, 150 },
        {277,  60 },
        {330,  40 },
        {345,  30 },
        { 77, 100 },
        { 91, 200 },
        {222, 300 }
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    char buf[128];
    int y = 20;
    MyDebug::instance().pGfx->text8x12(10, 10, "run_srp_dynamic_unittest_with_distance", Color::Cyan);

    SRP_DAS_Distance das;
    DistanceEstimator distEst(SDS_DIST_GAIN/91.025f, 1e-3f);

    for (int t = 0; t < numTests; ++t)
    {
        float trueAz   = tests[t].az;
        float trueDist = tests[t].dist;

        // 1) Testsignal erzeugen
        generateTestSignalForAzimuth(trueAz);
//        makeTestFrame360(trueAz, trueDist, 0.01f);
//        makeTestFrame360_farfield(trueAz, trueDist, 0.01f);

        // 2) SRP ausführen
        SRPPhat srp;
        MyDebug::instance().start();
        srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
        while (!srp.stepAzimuthScan()) {}
        float bestAz = srp.getResult();
        float azDiff = fabsf(bestAz - trueAz);

        // 3) DistanceEstimator
        SRPFrame frame = das.makeFrame(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP, 1000);
        DistanceResult dr = distEst.process(frame);
        float bestDist = dr.distance_m;
//        float bestConf = dr.confidence;
        float distDiff = fabsf(bestDist - trueDist);
        MyDebug::instance().stop();

        // 4) Ausgabe
        snprintf(buf, sizeof(buf),
                 "Az=%3.0f S=%3.0f dA=%4.1f D=%4.0f dD=%4.0f dt=%.3f %s",
                 trueAz, bestAz, azDiff,
                 bestDist, distDiff,
                 MyDebug::instance().diffMs(),
                 (azDiff < 2.0f && distDiff < 50.0f ? "OK" : "FAIL"));


        Color resultColor = Color::Red;
        if ((azDiff < 2.0f) && (distDiff < 5.0f) ) {
        	resultColor = Color::Green;
        } else if (azDiff < 2.0f) {
        	resultColor = Color::White;
        } else {
        	resultColor = Color::Red;
        }
        MyDebug::instance().pGfx->text8x12(10, y, buf, resultColor);
        y += 12;
    }
}

void UnitTest::run_srp_dynamic_unittest()
{
    const int testAngles[] = {0, 30, 45, 60, 90, 120, 277, 330, 345, 77, 91, 222, 304, 165, 166, 167, 204, 341};
    const int numTests = sizeof(testAngles) / sizeof(testAngles[0]);

    char buf[128];
    int y = 20;

    for (int t = 0; t < numTests; ++t)
    {
        float trueAz = testAngles[t];

        // 1) Testsignal erzeugen
//        generateTestSignalForAzimuth(trueAz);
//        makeTestFrame360(trueAz, 100.0f, 0.01f);
        makeTestFrame360_farfield(trueAz, 100.0f, 0.01f);

        // 2) SRP ausführen
        MyDebug::instance().start();
        SRPPhat srp;
        srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
        while (!srp.stepAzimuthScan()) {}
        float bestAz 	 = srp.getResult();
        float diff   	 = fabsf(bestAz - trueAz);
        MyDebug::instance().stop();

        // 3) Ausgabe
        snprintf(buf, sizeof(buf),
                 "Az=%3.0f  SRP=%3.0f  diff=%4.1f dt=%.3f %s",
                 trueAz, bestAz, diff, MyDebug::instance().diffMs(),
                 (diff <= 2.0f ? "OK" : "FAIL"));

        MyDebug::instance().pGfx->text8x12(10, y, buf, diff <= 2.0f ? Color::Green : Color::Red);
        y += 12;
    }
}

bool UnitTest::run_distance_unittest()
{
	int yOffset = 20;
    char buf[128];

    DistanceEstimator distEst(SDS_DIST_GAIN/91.025f, 1e-3f);
    SRP_DAS_Distance das;

    struct TestCase {
        float az;
        float dist;
    };

    const TestCase tests[] = {
        {  0,  50 },
        { 30,  80 },
        { 45, 120 },
        { 60, 200 },
        { 90, 300 },
        {120, 150 },
        {277,  60 },
        {330,  40 },
        {345,  30 },
        { 77, 100 },
        { 91, 200 },
        {222, 300 }
    };


    MyDebug::instance().pGfx->text8x12(10, 10, "run_distance_unittestxxxxxxxxxxxxxxx", Color::Cyan);
    for (auto& tc : tests)
    {
        // Testsignal erzeugen (deine bestehende makeTestFrame360 oder generateTestFrame)
        makeTestFrame360(tc.az, tc.dist, 0.01f);

        // SRP-DAS-Frame erzeugen
        SRPFrame frame = das.makeFrame(
            mic,
            SDS_AZ_MIN,
            SDS_AZ_MAX,
            SDS_AZ_STEP,
            1000
        );
        MyDebug::instance().start();
        DistanceResult dr = distEst.process(frame);
        MyDebug::instance().stop();

        snprintf(buf, sizeof(buf), "t=%.3f d=%3.2f, conf=%f dif%3.1f", MyDebug::instance().diffMs(), dr.distance_m, dr.confidence, 100.0f*(dr.distance_m-tc.dist)/tc.dist);
        MyDebug::instance().pGfx->text8x12(10, yOffset, buf, Color::White);
        yOffset += 12;
    }

    return true;
}

bool UnitTest::run_srp_unittest_1() {
	int yOffset = 10;
    char buf[128];

	SRPPhat srp;
    // 1) Testdaten laden (512 → 128 Samples)
    MyDebug::instance().start();
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int i = 0; i < SDS_FRAME_LEN; ++i)
            mic[ch][i] = csvData[ch][i];   // Downsampling
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,yOffset);
    yOffset += 10;

    // 2) SRP ausführen
    MyDebug::instance().start();
    srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,yOffset);
    yOffset += 10;

    MyDebug::instance().start();
    while (!srp.stepAzimuthScan()) {
        // optional debug
    }
    float bestAz = srp.getResult();
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,yOffset);
    yOffset += 10;

    snprintf(buf, sizeof(buf), "az: %3.2f, (dif %3.1f)", bestAz, bestAz-TEST_AZ);
    MyDebug::instance().pGfx->text8x12(10, yOffset, buf, Color::White);
    yOffset += 10;

    SRPFrame frame{};
    frame.srp_grid     = srp.getGrid();
    frame.ntheta       = 361; //srp.getNtheta();
    frame.dtheta_deg   = SDS_AZ_STEP;
    frame.azimuth_deg  = bestAz;
    frame.timestamp_ms = HAL_GetTick();
    frame.valid        = true;
    snprintf(buf, sizeof(buf), "%p, %d, %3.0f, %.1f, %lu %d",frame.srp_grid, frame.ntheta, frame.dtheta_deg, frame.azimuth_deg, frame.timestamp_ms, frame.valid );
    MyDebug::instance().pGfx->text8x12(10, yOffset, buf, Color::White);
    yOffset += 20;


    // TrackingTask AUFRUFEN
    MyDebug::instance().start();
	TrackingTask tracker;
	DistanceEstimator dist(10.0f, 100.0f);
    tracker.process(frame, dist);
    // Ergebnis holen
    TrackState ts = tracker.getLastState();
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,yOffset);
    yOffset += 10;

    // Anzeigen
    snprintf(buf, sizeof(buf),
             "Dist=%.1f Conf=%.2f Az=%.1f",
             ts.distance_m, ts.confidence, ts.azimuth_deg);
    MyDebug::instance().pGfx->text8x12(10, yOffset, buf, Color::White);
    yOffset += 10;

    const float tolerance = 1.0f;
    bool pass = std::fabs(bestAz - TEST_AZ) < tolerance;
    return pass;
}

bool UnitTest::run_srp_unittest() {
    SRPPhat srp;
    // 1) Testdaten laden (512 → 128 Samples)
    MyDebug::instance().start();
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int i = 0; i < SDS_FRAME_LEN; ++i)
            mic[ch][i] = csvData[ch][i];   // Downsampling
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,20);

    // 2) SRP ausführen
    MyDebug::instance().start();
    srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,30);

    MyDebug::instance().start();
    while (!srp.stepAzimuthScan()) {
        // optional debug
    }

    float bestAz = srp.getResult();
    MyDebug::instance().stop();
    MyDebug::instance().deltaTimeMs(10,40);

    char buf[64];
    snprintf(buf, sizeof(buf), "az: %3.2f, (dif %3.1f)", bestAz, bestAz-TEST_AZ);
    MyDebug::instance().pGfx->text8x12(10, 50, buf, Color::White);

    const float expected  = TEST_AZ;
    const float tolerance = 1.0f;
    bool pass = std::fabs(bestAz - expected) < tolerance;
    return pass;
}

bool UnitTest::run_srp_fast_unittest()
{
//    SRPPhat srp;
//
//    // Testdaten: 512 → 128 Samples (einfach erstes Segment)
//    MyDebug::instance().start();
//    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
//        for (int i = 0; i < SDS_FRAME_LEN; ++i)
//            mic[ch][i] = csvData[ch][i];
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,70);
//
//    // Grob + fein: hier einfach 0–360 in SDS_AZ_STEP
//    MyDebug::instance().start();
//    srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,80);
//
//    MyDebug::instance().start();
//    while (!srp.stepAzimuthScan()) {}
//
//    float bestAz = srp.getResult();
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,90);
//
//    char buf[64];
//    snprintf(buf, sizeof(buf), "az: %3.2f, (dif %3.1f)", bestAz, bestAz-TEST_AZ);
//    MyDebug::instance().pGfx->text8x12(10, 100, buf, Color::White);
//
//    const float expected  = TEST_AZ;
//    const float tolerance = 5.0f; // etwas großzügiger wegen Peak-Only
//    bool pass = std::fabs(bestAz - expected) < tolerance;
//    return pass;
	return true;
}

bool UnitTest::run_srp_old_unittest()
{
//    SRPPhatOld srp;
//    MyDebug::instance().start();
//    for (int ch = 0; ch < SDS_NUM_MICS; ++ch) {
//        for (int i = 0; i < SDS_FRAME_LEN; ++i) {
//            mic[ch][i] = csvData[ch][i];   // Downsampling
//        }
//    }
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,20);
//
//    MyDebug::instance().start();
//    srp.beginAzimuthScan(mic, SDS_AZ_MIN, SDS_AZ_MAX, SDS_AZ_STEP);
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,30);
//
//    MyDebug::instance().start();
//    while (!srp.stepAzimuthScan()) {
//
//    }
//    float bestAz = srp.getResult();
//    MyDebug::instance().stop();
//    MyDebug::instance().deltaTimeMs(10,40);
//
//    char buf[64];
//    snprintf(buf, sizeof(buf), "az: %3.2f, (dif %3.1f)", bestAz, bestAz-TEST_AZ);
//    MyDebug::instance().pGfx->text8x12(10, 50, buf, Color::White);
//
//    const float expected  = TEST_AZ;
//    const float tolerance = 1.0f;
//    return std::fabs(bestAz - expected) < tolerance;
	return true;
}

// Erzeugt ein synthetisches Testsignal für einen gegebenen Azimut.
// mic[ch][i] wird vollständig gefüllt.
// Keine externen Daten nötig.
void UnitTest::generateTestSignalForAzimuth(float azDeg)
{
    const float c = 343.0f;                 // Schallgeschwindigkeit
    const float fs = SDS_SAMPLE_RATE;       // 48000 Hz
    const float inv_c_fs = fs / c;

    // Azimut → Richtungseinheitsvektor
    float azRad = azDeg * (3.1415926535f / 180.0f);
    float dx = cosf(azRad);
    float dy = sinf(azRad);

    // Position des Impulses im Frame
    const int impulsePos = 40;

    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        // Echte Mikrofonpositionen aus SDS_Params.hpp
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        // Delay in Samples (Projektion auf Einfallsrichtung)
        float proj = x * dx + y * dy;
        int delaySamples = (int)(proj * inv_c_fs);

        // Buffer löschen
        for (int i = 0; i < SDS_FRAME_LEN; ++i)
            mic[ch][i] = 0.0f;

        // Impuls setzen
        int pos = impulsePos + delaySamples;
        if (pos >= 0 && pos < SDS_FRAME_LEN)
            mic[ch][pos] = 1.0f;
    }
}

void UnitTest::generateTestSignalForAzimuthAndDistance(float azDeg, float distanceMeters)
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;
    const float inv_c_fs = fs / c;

    float azRad = azDeg * (3.1415926535f / 180.0f);
    float dx = cosf(azRad);
    float dy = sinf(azRad);

    const int impulsePos = 40;

    // 1/r-Dämpfung
    float amp = 1.0f / (distanceMeters + 1.0f);

    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        float proj = x * dx + y * dy;
        int delaySamples = (int)(proj * inv_c_fs);

        for (int i = 0; i < SDS_FRAME_LEN; ++i)
            mic[ch][i] = 0.0f;

        int pos = impulsePos + delaySamples;
        if (pos >= 0 && pos < SDS_FRAME_LEN)
            mic[ch][pos] = amp;
    }
}

// angleDeg  : Einfallswinkel in Grad (0° = +x-Achse)
// distance_m: Distanz in Metern (50..200 m)
// noise     : Rauschanteil (z.B. 0.01f für 1% des Signals)
void UnitTest::generateTestFrame(float angleDeg, float distance_m, float noise)
{
    const float c      = 343.0f;              // m/s
    const float fs     = SDS_SAMPLE_RATE;     // Hz
    const float radius = SDS_MIC_RADIUS;      // m

    // Fernfeld-Amplitude (1/r)
    const float k_ref  = 5.0f;
    float A = k_ref / distance_m;

    float angleRad = angleDeg * (3.1415926535f / 180.0f);
    float dx = std::cos(angleRad);
    float dy = std::sin(angleRad);

    int n0 = SDS_FRAME_LEN / 2;

    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            mic[ch][n] = 0.0f;

    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float phi = 2.0f * 3.1415926535f * ch / SDS_NUM_MICS;
        float x   = radius * std::cos(phi);
        float y   = radius * std::sin(phi);

        float proj = x * dx + y * dy;
        float tau  = proj / c;
        int delaySamples = static_cast<int>(tau * fs);

        int idx = n0 + delaySamples;
        if (idx >= 0 && idx < SDS_FRAME_LEN)
            mic[ch][idx] = A;

        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            float r = (std::rand() / (float)RAND_MAX) - 0.5f;
            mic[ch][n] += noise * A * r;
        }
    }
}

//#define SDS_ALTE_STRATEGY
#ifdef SDS_ALTE_STRATEGY
// mic: das gleiche Array, das du später an SRPPhatOld übergibst
void UnitTest::makeTestFrame360(float angleDeg,
                                float distance_m,
                                float noise)
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;

    // Fernfeld-Amplitude, abgestimmt auf SDS_DIST_GAIN
    const float k_ref = SDS_DIST_GAIN;      // 20000.0f
    float A = k_ref / distance_m;

    float theta = angleDeg * (3.1415926535f / 180.0f);
    int n0 = SDS_FRAME_LEN / 2;

    // Array leeren
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            mic[ch][n] = 0.0f;

    // für jedes Mic: Position aus SDS_MIC_POSITIONS, Delay + Breitband-Burst
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        // Einfallsrichtung als Einheitsvektor
        float dx = std::cos(theta);
        float dy = std::sin(theta);

        // Projektion → Laufzeitunterschied
        float proj = x * dx + y * dy;   // Meter
        float tau  = proj / c;          // Sekunden
        int delaySamples = static_cast<int>(tau * fs);

        // kurzer Breitband-Burst um die Delay-Position
        for (int k = -4; k <= 4; ++k)
        {
            int idx = n0 + delaySamples + k;
            if (idx >= 0 && idx < SDS_FRAME_LEN)
            {
                float r = (std::rand() / (float)RAND_MAX) - 0.5f;
                mic[ch][idx] += A * r;
            }
        }

        // Hintergrundrauschen
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            float r = (std::rand() / (float)RAND_MAX) - 0.5f;
            mic[ch][n] += noise * A * r;
        }
    }
}

#else
void UnitTest::makeTestFrame360(float angleDeg, float distance_m, float noise)
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;

    // Druckamplitude ∝ 1/r
    const float A = 1.0f / distance_m;

    float theta = angleDeg * (M_PI / 180.0f);
    float dx = std::cos(theta);
    float dy = std::sin(theta);

    // Frame leeren
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            mic[ch][n] = 0.0f;

    // Breitband-Rauschsignal mit kohärenter Delay-Geometrie
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        float tau = (x*dx + y*dy) / c;
        int delaySamples = int(tau * fs);

        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            int idx = n + delaySamples;
            if (idx >= 0 && idx < SDS_FRAME_LEN)
            {
                float r = (std::rand() / float(RAND_MAX)) - 0.5f;
                mic[ch][idx] += A * r;
            }
        }

        // Hintergrundrauschen
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            float r = (std::rand() / float(RAND_MAX)) - 0.5f;
            mic[ch][n] += noise * A * r;
        }
    }
}

#endif

void UnitTest::makeTestFrame360_farfield(float angleDeg, float distance_m, float noise)
{
    const float c  = 343.0f;
    const float fs = SDS_SAMPLE_RATE;

    float theta = angleDeg * (M_PI / 180.0f);
    float dx = std::cos(theta);
    float dy = std::sin(theta);

    float A = 1.0f / distance_m;

    // Frame leeren
    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            mic[ch][n] = 0.0f;

    for (int ch = 0; ch < SDS_NUM_MICS; ++ch)
    {
        float x = SDS_MIC_POSITIONS[ch][0];
        float y = SDS_MIC_POSITIONS[ch][1];

        // Fernfeld: Delay = Projektion
        float tau = (x*dx + y*dy) / c;
        float delay = tau * fs;        // fractional delay

        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            float t = n - delay;

            // Breitband-Rauschsignal mit fractional delay
            float r = (std::rand() / float(RAND_MAX)) - 0.5f;

            // Linear interpolation (kann später durch FIR ersetzt werden)
            int i0 = int(t);
            float frac = t - i0;

            if (i0 >= 0 && i0+1 < SDS_FRAME_LEN)
            {
                float s = (1-frac)*r + frac*r; // Dummy source
                mic[ch][n] += A * s;
            }
        }

        // Hintergrundrauschen
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
        {
            float r = (std::rand() / float(RAND_MAX)) - 0.5f;
            mic[ch][n] += noise * A * r;
        }
    }
}
