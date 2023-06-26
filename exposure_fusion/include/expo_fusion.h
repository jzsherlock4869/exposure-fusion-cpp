#ifndef EXPO_FUSION_H
#define EXPO_FUSION_H

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <vector>

// params for exposure fusion
typedef struct ExpoFusionConfig{
    int pyrLevel;
    float saturationIndex;
    float contrastIndex;
    float wellExpoIndex;
    int contrastKernelSize;
    float wellExpoMean;
    float wellExpoSigma;
    float whitePercent;
    float blackPercent;
} ExpoFusionConfig;

void initExpoFusionConfig(ExpoFusionConfig &config);
cv::Mat expoFusion(std::vector<cv::Mat> images, ExpoFusionConfig config);

#endif