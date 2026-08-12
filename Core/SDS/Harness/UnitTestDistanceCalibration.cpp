#include "UnitTestDistanceCalibration.hpp"
#include "SDS_Params.hpp"
#include "MyDebug.hpp"
#include "UnitTestSignals.hpp"
#include <cmath>

static int rowY = 20;

// ---------------------------------------------------------
//  KORREKTE NICHTLINEARE KALIBRIERUNG
//  C = mean(trueDist * rms)
// ---------------------------------------------------------

float UnitTestDistanceCalibration::estimateDistance(const float mic[8][SDS_FRAME_LEN])
{
    float sum = 0.0f;

    for (int ch = 0; ch < 8; ++ch)
        for (int n = 0; n < SDS_FRAME_LEN; ++n)
            sum += mic[ch][n] * mic[ch][n];

    float rms = std::sqrt(sum / (8 * SDS_FRAME_LEN));

    float est = SDS_DIST_GAIN / rms;
    return est;
}

void UnitTestDistanceCalibration::runCalibration()
{
    rowY = 20;

//    MyDebug::instance().lcdPrint(10, rowY, "Distanz-Kalibrierung");
    rowY += 14;

    float mic[8][SDS_FRAME_LEN];
    std::vector<Result> results;

    float sumC = 0.0f;
    int count  = 0;

    for (float dist = 50.0f; dist <= 200.0f; dist += 25.0f)
    {
        UnitTestSignals::makeTestFrame360_farfield(
            0.0f,
            dist,
            0.05f,
            mic);

        // RMS berechnen
        float sum = 0.0f;
        for (int ch = 0; ch < 8; ++ch)
            for (int n = 0; n < SDS_FRAME_LEN; ++n)
                sum += mic[ch][n] * mic[ch][n];

        float rms = std::sqrt(sum / (8 * SDS_FRAME_LEN));

        // Hyperbolische Kalibrierung: C = mean(dist * rms)
        sumC += dist * rms;
        count++;

        float est = estimateDistance(mic);

//        MyDebug::instance().lcdPrint(
//            10, rowY,
//            "Dist %.1f m, rms=%.6f, est=%.1f m",
//            dist, rms, est);
        rowY += 14;

        results.push_back({dist, est});
    }

//    float C_opt = sumC / count;

//    MyDebug::instance().lcdPrint(10, rowY, "Optimaler Gain C = %.4f", C_opt);
    rowY += 14;

//    MyDebug::instance().lcdPrint(10, rowY, "Fertig.");
    rowY += 14;
}
