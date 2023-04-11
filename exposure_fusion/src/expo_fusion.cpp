#include "expo_fusion.h"
#include "pyramid_ops.h"
#include "calc_weight.h"

void initExpoFusionConfig(ExpoFusionConfig &config){

    config.pyrLevel = 9;

    config.saturationIndex = 1.f;
    config.contrastIndex = 1.f;
    config.wellExpoIndex = 1.f;

    config.contrastKernelSize = 11;
    config.wellExpoMean = 0.5f;
    config.wellExpoSigma = 0.2f;
}

cv::Mat expoFusion(std::vector<cv::Mat> images, ExpoFusionConfig config){
    
    // calc weight maps
    std::vector<cv::Mat> mergeWeightMaps = getMergeWeights(images, config);
    std::vector<cv::Mat> curWeightPyr, curLapPyr, outputPyr;
    cv::Mat curWeight, curLap, curWeight3c, curMerged;

    printf("[expoFusion] num of images to be fused : %d \n", (int)images.size());
    for(int i = 0; i < mergeWeightMaps.size(); i++){
        // build gaussian pyr for weight maps
        curWeightPyr = buildGaussianPyr(mergeWeightMaps.at(i), config.pyrLevel);
        // build laplacian pyr for images
        curLapPyr = buildLaplacianPyr(images.at(i), config.pyrLevel);
        // merge using gaussian pyr of weights and laplacian pyr of images
        for(int layer = 0; layer < config.pyrLevel; layer ++){
            curWeight = curWeightPyr.at(layer);
            cv::cvtColor(curWeight, curWeight3c, cv::COLOR_GRAY2RGB);
            curLap = curLapPyr.at(layer);
            if (0 == i){
                outputPyr.push_back(curWeight3c.mul(curLap));
            }
            else{
                outputPyr.at(layer) += curWeight3c.mul(curLap);
            }
            double minw, maxw;
            cv::minMaxLoc(outputPyr.at(layer), &minw, &maxw);
            printf("[expoFusion] imgIdx=%d, layerIdx=%d, size=%dx%d, min=%lf, max=%lf \n", i, layer, outputPyr.at(layer).rows, outputPyr.at(layer).cols, minw, maxw);
        }
        printf("[expoFusion] the %dth image fused. \n", i);
    }
    printf("[expoFusion] fusion completed, start collapsing ... \n");
    // collapse merged laplacian pyrs to form fusion image
    cv::Mat outputImage = collapseLaplacianPyr(outputPyr);
    return outputImage;
    }