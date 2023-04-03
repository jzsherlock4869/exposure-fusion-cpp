#ifndef PYRAMID_OPS_H
#define PYRAMID_OPS_H

#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <vector>

std::vector<cv::Mat> buildGaussianPyr(cv::Mat src, int numLevel);
std::vector<cv::Mat> buildLaplacianPyr(cv::Mat src, int numLevel);
std::vector<cv::Mat> convertGaussianToLaplacianPyr(std::vector<cv::Mat> gaussPyr);
cv::Mat collapseLaplacianPyr(std::vector<cv::Mat> laplacianPyr);

#endif