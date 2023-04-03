#ifndef CALC_WEIGHT_H
#define CALC_WEIGHT_H

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <vector>
#include <math.h>
#include "expo_fusion.h"

float getExponentialWeight(float val, float mean, float sigma);
cv::Mat calcSaturationMap(cv::Mat img);
cv::Mat calcContrastMap(cv::Mat img, int kernelSize);
cv::Mat calcWellExpoMap(cv::Mat img, float mean, float sigma);
cv::Mat calcWeightMap(cv::Mat img, float satuIdx, float conIdx, float wellIdx, int kernelSize, float mean, float sigma);
std::vector<cv::Mat> getMergeWeights(std::vector<cv::Mat> imgs, ExpoFusionConfig config);

#endif