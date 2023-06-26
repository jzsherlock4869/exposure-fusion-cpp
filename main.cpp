#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "exposure_fusion/include/pyramid_ops.h"
#include "exposure_fusion/include/calc_weight.h"
#include "exposure_fusion/include/expo_fusion.h"

std::string rootDir;

std::string dirnameOf(const std::string& fname){
    size_t pos = fname.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : fname.substr(0, pos);
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("usage: ExposureFusion.out <ImagePath1> <ImagePath2> [<ImagePath3> ...] \n");
        return -1;
    }

    std::vector<std::string> allPaths;
    allPaths.assign(argv + 1, argv + argc);
    printf("[main] num of exposures : %d \n", (int)allPaths.size());
    rootDir = dirnameOf(dirnameOf(allPaths.at(0))) + "/result";

    std::vector<cv::Mat> images_f;
    cv::Mat tmpImg;
    for(int i = 0; i < allPaths.size(); i ++){
        std::string curPath = allPaths.at(i);
        tmpImg = cv::imread(curPath, 1);
        printf("[main] load %dth image, current path is %s \n", i, curPath.c_str());
        printf("       depth: %d channels: %d size: %dx%d \n", tmpImg.depth(), tmpImg.channels(), tmpImg.rows, tmpImg.cols);
        if (!tmpImg.data) {
            printf("[!Error!] [main] No image data in %dth filepath, please check !\n", i);
            return -1;
        }
        tmpImg.convertTo(tmpImg, CV_32FC3, 1/255.f);
        images_f.push_back(tmpImg);
    }

    ExpoFusionConfig config;
    initExpoFusionConfig(config);

    // constraint pyramid level to be <= max level log2(min(h, w))
    float min_len = std::min(float(images_f.at(0).cols), float(images_f.at(0).rows));
    int maxPyrLevel = floor(log2(min_len));
    if(maxPyrLevel < config.pyrLevel){
        printf("[main] maxPyrLevel %d < config.PyrLevel %d, set to max \n",
                        maxPyrLevel, config.pyrLevel);
        config.pyrLevel = maxPyrLevel;
    }

    cv::Mat fusedImage = expoFusion(images_f, config);
    double minw, maxw;
    cv::minMaxLoc(fusedImage, &minw, &maxw);
    printf("[main] fused image size: %d %d, min: %f, max: %f \n", fusedImage.rows, fusedImage.cols, (float)minw, (float)maxw);


    cv::Mat fusedImage_8u;
    fusedImage.convertTo(fusedImage_8u, CV_8UC3, 255.f);
    cv::minMaxLoc(fusedImage_8u, &minw, &maxw);
    printf("[main] saved uint8 image max %d min %d \n", (int)maxw, (int)minw);
    std::string dstPath = rootDir + "/fused_image.png";
    cv::imwrite(dstPath, fusedImage_8u);

    // const std::string windowName = "fused image";
    // cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    // cv::imshow(windowName, fusedImage_8u);
    // cv::waitKey(0);

    return 0;
}
