#include "pyramid_ops.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

// generate Gaussian pyramid for image
std::vector<cv::Mat> buildGaussianPyr(cv::Mat src, int numLevel){
    std::vector<cv::Mat> pyr;
    pyr.push_back(src);
    cv::Mat curIn, curOut;
    curIn = src;
    int new_cols, new_rows;
    for(int level = 1; level < numLevel; level++){
        new_cols = round(curIn.cols / 2.0);
        new_rows = round(curIn.rows / 2.0);
        cv::pyrDown(curIn, curOut, cv::Size(new_cols, new_rows));
        pyr.push_back(curOut);
        curIn = curOut;
    }
    return pyr;
}

// generate Laplacian pyramid for image
std::vector<cv::Mat> buildLaplacianPyr(cv::Mat src, int numLevel){
    std::vector<cv::Mat> pyr;
    cv::Mat curIn, curOut, curUp;
    curIn = src;
    int new_cols, new_rows;
    for(int level = 0; level < numLevel - 1; level++){
        new_cols = round(curIn.cols / 2.0);
        new_rows = round(curIn.rows / 2.0);
        cv::pyrDown(curIn, curOut, cv::Size(new_cols, new_rows));
        cv::pyrUp(curOut, curUp, cv::Size(curIn.cols, curIn.rows));
        pyr.push_back(curIn - curUp);
        curIn = curOut;
    }
    pyr.push_back(curOut);
    return pyr;
}

// get Laplacian pyramid from Gaussian pyramid
std::vector<cv::Mat> convertGaussianToLaplacianPyr(std::vector<cv::Mat> gaussPyr){
    int numLevel = gaussPyr.size();
    std::vector<cv::Mat> laplacianPyr;
    cv::Mat curIn, curUp;
    for(int level = 0; level < numLevel - 1; level++){
        curIn = gaussPyr.at(level);
        cv::pyrUp(gaussPyr.at(level + 1), curUp, cv::Size(curIn.cols, curIn.rows));
        laplacianPyr.push_back(curIn - curUp);
    }
    laplacianPyr.push_back(gaussPyr.at(numLevel - 1));
    return laplacianPyr;
}

// collapse Laplacian pyramid to form an image
cv::Mat collapseLaplacianPyr(std::vector<cv::Mat> laplacianPyr){
    int numLevel = laplacianPyr.size();
    cv::Mat curTmp, curUp, curOut;
    curOut = laplacianPyr.at(numLevel - 1);
    for(int level = numLevel - 2; level >= 0; level--){
        curTmp = laplacianPyr.at(level);
        cv::pyrUp(curOut, curUp, cv::Size(curTmp.cols, curTmp.rows));
        curOut = curUp + curTmp;
        double minw, maxw;
        cv::minMaxLoc(curOut, &minw, &maxw);
        printf("[collapseLaplacianPyr] current level=%d, min=%f, max=%f \n", level, (float)minw, (float)maxw);

    }
    return curOut;
}
