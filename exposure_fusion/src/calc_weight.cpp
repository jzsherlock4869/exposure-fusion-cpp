#include "calc_weight.h"

extern std::string rootDir;

// calc well-exposedness with exponential weight
float getExponentialWeight(float val, float mean, float sigma){
    float weight = exp(-1.f * pow(val - mean, 2) / (2.f * pow(sigma, 2)));
    return weight;
}

cv::Mat calcSaturationMap(cv::Mat img){
    // img should be in type CV_32F3 with range 0-1
    cv::Mat satuMap(cv::Size(img.cols, img.rows), CV_32FC1);
    float R, G, B, avg, satuV;
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            R = img.at<cv::Vec3f>(i, j)[0];
            G = img.at<cv::Vec3f>(i, j)[1];
            B = img.at<cv::Vec3f>(i, j)[2];
            avg = (R + G + B) / 3.f;
            satuV = sqrt((pow(R - avg, 2) + pow(G - avg, 2) + pow(B - avg, 2)) / 3.f);
            satuMap.at<float>(i, j) = satuV;
        }
    }
    return satuMap;
}

cv::Mat calcContrastMap(cv::Mat img, int kernelSize){
    cv::Mat imgGray, lapMap, conMap;
    cv::cvtColor(img, imgGray, cv::COLOR_RGB2GRAY);
    // GaussianBlur(imgGray, imgGray, cv::Size(3, 3), 0);
    cv::Laplacian(imgGray, lapMap, -1, 1);
    lapMap = cv::abs(lapMap);
    cv::boxFilter(lapMap, conMap, -1, cv::Size(kernelSize, kernelSize), cv::Point(-1, -1), false);
    double minV, maxV;
    cv::minMaxLoc(conMap, &minV, &maxV);
    conMap = (conMap - (float)minV) / ((float)maxV - (float)minV);
    return conMap;
}

cv::Mat calcWellExpoMap(cv::Mat img, float mean, float sigma){
    cv::Mat wellMap(cv::Size(img.cols, img.rows), CV_32FC1);
    float R, G, B;
    float wR, wG, wB;
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            R = img.at<cv::Vec3f>(i, j)[0];
            G = img.at<cv::Vec3f>(i, j)[1];
            B = img.at<cv::Vec3f>(i, j)[2];
            wR = getExponentialWeight(R, mean, sigma);
            wG = getExponentialWeight(G, mean, sigma);
            wB = getExponentialWeight(B, mean, sigma);
            wellMap.at<float>(i, j) = wR * wG * wB;
        }
    }
    return wellMap;
}

cv::Mat calcWeightMap(cv::Mat img, float satuIdx, float conIdx, float wellIdx, 
                      int kernelSize, float mean, float sigma){
    cv::Mat weightMap;
    cv::Mat satuMap, conMap, wellMap;
    cv::pow(calcSaturationMap(img), satuIdx, satuMap);
    cv::pow(calcContrastMap(img, kernelSize), conIdx, conMap);
    cv::pow(calcWellExpoMap(img, mean, sigma), wellIdx, wellMap);
    weightMap = satuMap.mul(conMap).mul(wellMap) + 1e-8f;
    return weightMap;
}


std::vector<cv::Mat> getMergeWeights(std::vector<cv::Mat> imgs, ExpoFusionConfig config){

    float satuIdx = config.saturationIndex;
    float conIdx = config.contrastIndex;
    float wellIdx = config.wellExpoIndex;
    int kernelSize = config.contrastKernelSize;
    float mean = config.wellExpoMean;
    float sigma = config.wellExpoSigma;

    int idx = 0;
    std::vector<cv::Mat> weightMaps;
    cv::Mat tmpW;
    for(idx = 0; idx < imgs.size(); idx++){
        cv::Mat img = imgs.at(idx);
        tmpW = calcWeightMap(img, satuIdx, conIdx, wellIdx, kernelSize, mean, sigma);
        weightMaps.push_back(tmpW);
    }
    cv::Mat weightSum(cv::Size(tmpW.cols, tmpW.rows), CV_32FC1, cv::Scalar(0));
    for(idx = 0; idx < weightMaps.size(); idx++){
        weightSum += weightMaps.at(idx);
    }

    double minV, maxV;
    cv::minMaxLoc(weightSum, &minV, &maxV);
    printf("[getMergeWeights] original weightSum min %lf, max %lf \n", minV, maxV);

    cv::Mat tmp_f, tmp_8u;
    for(idx = 0; idx < weightMaps.size(); idx++){
        weightMaps.at(idx) = weightMaps.at(idx) / (weightSum + 1e-8f);
        // // visualize weight maps
        // weightMaps.at(idx).convertTo(tmp_f, CV_32FC3, 255.f);
        // cv::cvtColor(tmp_f, tmp_8u, cv::COLOR_GRAY2RGB);
        // std::string dstPath = rootDir + "/weight_" + std::to_string(idx) + ".png";
        // cv::imwrite(dstPath, tmp_8u);
    }

    // 2nd weight normalization to reduce calc error
    for(idx = 0; idx < weightMaps.size(); idx++){
        weightSum += weightMaps.at(idx);
    }

    for(idx = 0; idx < weightMaps.size(); idx++){
        weightMaps.at(idx) = weightMaps.at(idx) / (weightSum);
        // visualize weight maps
        weightMaps.at(idx).convertTo(tmp_f, CV_32FC3, 255.f);
        cv::cvtColor(tmp_f, tmp_8u, cv::COLOR_GRAY2RGB);
        std::string dstPath = rootDir + "/weight_" + std::to_string(idx) + ".png";
        cv::imwrite(dstPath, tmp_8u);
    }

    return weightMaps;
}

cv::Mat robustNorm(cv::Mat img, float whitePercent, float blackPercent){
    
    // ref. to An Implementation of the Exposure Fusion Algorithm, Charles Hessel
    // Section 5: Clipping and Robust Normalization
    // https://www.ipol.im/pub/art/2018/230/article_lr.pdf
    
    std::vector<cv::Mat> rgb;
    cv::split(img, rgb);

    cv::Mat minChannel, maxChannel;
    minChannel = cv::min(rgb.at(0), rgb.at(1));
    minChannel = cv::min(minChannel, rgb.at(2));
    maxChannel = cv::max(rgb.at(0), rgb.at(1));
    maxChannel = cv::max(minChannel, rgb.at(2));

    int maxNum = int(whitePercent * img.cols * img.rows);
    int minNum = int(blackPercent * img.cols * img.rows);
    maxNum = std::max(1, maxNum);
    minNum = std::max(1, minNum);
    printf("[robustNorm] clip minNum : %d, maxNum : %d \n", minNum, maxNum);

    std::vector<float> pixelValues(img.begin<float>(), img.end<float>());
    std::vector<float> maxValues(maxNum);
    std::vector<float> minValues(minNum);

    std::partial_sort_copy(pixelValues.begin(), pixelValues.end(),
                           maxValues.begin(), maxValues.end(), std::greater<float>());
    std::partial_sort_copy(pixelValues.begin(), pixelValues.end(),
                           minValues.begin(), minValues.end());
    std::nth_element(maxValues.begin(), maxValues.begin() + (maxNum - 1), maxValues.end(), std::greater<int>());
    std::nth_element(minValues.begin(), minValues.begin() + (minNum - 1), minValues.end());

    float clip_max = maxValues.back();
    float clip_min = minValues.front();
    printf("[robustNorm] clip min : %f, clip max %f \n", clip_min, clip_max);

    cv::Mat normImg = (img - cv::Scalar::all(clip_min)) / (clip_max - clip_min);

    double minw, maxw;
    cv::minMaxLoc(normImg, &minw, &maxw);
    printf("[main] normImg image size: %d %d, min: %f, max: %f \n", normImg.rows, normImg.cols, (float)minw, (float)maxw);

    cv::threshold(normImg, normImg, 1.0, 1.0, cv::THRESH_TRUNC);
    cv::threshold(-normImg, normImg, 0.0, 0.0, cv::THRESH_TRUNC);
    normImg = - normImg;

    cv::minMaxLoc(normImg, &minw, &maxw);
    printf("[robustNorm] thres normImg image size: %d %d, min: %f, max: %f \n", normImg.rows, normImg.cols, (float)minw, (float)maxw);

    return normImg;
}

