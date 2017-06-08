#include <iostream>
#include "Communication.h"

int main() {
    Client client;
    cv::Mat cameraFeed = cv::imread("image.jpg", cv::IMREAD_COLOR);
//    cv::Mat cameraFeed = cv::imread("depth.png", cv::IMREAD_ANYDEPTH);
    if (!cameraFeed.data)
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }
//    cameraFeed.convertTo(cameraFeed,CV_32F,1/256.0);
    double min, max;
    cv::Point minLoc;
    cv::Point maxLoc;
    cameraFeed = cameraFeed.reshape(1);
    cv::minMaxLoc(cameraFeed, &min, &max, &minLoc, &maxLoc );
    std::cout << "Min: " << min << " Max: " << max << std::endl;
    std::vector<cv::Mat> images;
    for (int i = 0; i < 5; i++)
    {
        images.push_back(cameraFeed.clone());
    }
    client.sendImages(images);
    Client::ClsPosPairs clsPosPairs;
    std::cout<< "Sending Done"<<std::endl;
    client.getSegResult(clsPosPairs);
    std::cout<< "==================================="<<std::endl;
    std::cout<< "Response:"<<std::endl;
    for (int i = 0; i < clsPosPairs.size(); i++)
    {
        std::cout<< "Name: " << clsPosPairs[i].first << std::endl;
        std::cout<< "Position: " << std::setw(6) << std::fixed << std::setprecision(3);
        std::vector<double> pos = clsPosPairs[i].second;
        for (int j = 0; j < pos.size(); j++)
        {
            std::cout<< pos[j] << " ";
        }
        std::cout << std::endl << std::endl;
    }

    return 0;
}