#include <iostream>
#include "Communication.h"

int send(Client& client)
{
    cv::Mat cameraFeed = cv::imread("image.jpg", cv::IMREAD_COLOR);
    if (!cameraFeed.data)
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }
    std::cout<<"height: "<<cameraFeed.rows<< " width: " << cameraFeed.cols << " type: "<< cameraFeed.type() << std::endl;

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
    return 0;
}
int main() {
    Client client;
    send(client);
    return 0;
}