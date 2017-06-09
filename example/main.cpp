#include <iostream>
#include "Communication.h"
#include <chrono>
#include <ctime>

#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

int send(Client& client)
{
    cv::Mat color_img = cv::imread("ImgColor2.png", cv::IMREAD_COLOR);
    if (!color_img.data)
    {
        std::cout <<  "Could not open or find the color image" << std::endl ;
        return -1;
    }
    std::vector<cv::Mat> color_imgs;
    color_imgs.push_back(color_img);

    cv::Mat depth_img = cv::imread("ImgDepth2.png", cv::IMREAD_ANYDEPTH);
    if (!depth_img.data)
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }
    depth_img.convertTo(depth_img,CV_32F,1/1000.0);

//    double min, max;
//    cv::Point minLoc;
//    cv::Point maxLoc;
//    cv::Mat cameraFeedFlat = depth_img.reshape(1);
//    cv::minMaxLoc(cameraFeedFlat, &min, &max, &minLoc, &maxLoc );
//    std::cout << "Min: " << min << " Max: " << max << std::endl;

    std::vector<cv::Mat> depth_imgs;
    depth_imgs.push_back(depth_img);

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
    start = std::chrono::system_clock::now();
    std::cout<< std::endl << "==================================="<<std::endl;
    client.sendImages(color_imgs);
    client.sendImages(depth_imgs);

    Client::ClsPosPairs clsPosPairs;
    std::cout<< "Sending Done"<<std::endl;
    client.getSegResult(clsPosPairs);

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << FGRN("Finished computation at ") << std::ctime(&end_time)
              << FGRN("Elapsed time: ") << elapsed_seconds.count() << "s\n";


    std::cout<< FBLU("Response:")<<std::endl;
    for (int i = 0; i < clsPosPairs.size(); i++)
    {
        std::cout<< "Name: " << clsPosPairs[i].first << std::endl;
        std::cout<< "Position: " << std::setw(6) << std::fixed << std::setprecision(3) << std::endl;
        std::vector<std::vector<double> > poses = clsPosPairs[i].second;
        for (int j = 0; j < poses.size(); j++)
        {
            std::vector<double> pos = poses[j];
            std::cout<< "          ";
            for (int k = 0; k < pos.size(); k++)
            {
                std::cout << pos[k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

    }
    std::cout << "==================================="<<std::endl;
    return 0;
}
int main() {
    Client client;
    for (int i = 0; i < 10; i++)
    {
        send(client);
    }

    return 0;
}