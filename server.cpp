#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


#define PORT 7200


void error(const char *msg)
{
  perror(msg);
  exit(1);
}

bool recv_all(int socket, void *buffer, int length)
{
  uchar *ptr = (uchar*) buffer;
  int bytes = 0;
  while (length > 0)
  {
    bytes = recv(socket, ptr, length, 0);
    if (bytes < 1) return false;
    ptr += bytes;
    length -= bytes;
  }
  return true;
}
bool send_all(int socket, void *buffer, int length)
{
    uchar *ptr = (uchar*) buffer;
    int bytes = 0;
    while (length > 0)
    {
        bytes = send(socket, ptr, length, 0);
        if (bytes < 1) return false;
        ptr += bytes;
        length -= bytes;
    }
    return true;
}

int main()
{
  int sockfd, newsockfd, portno, n, imgSize, bytes = 0, IM_HEIGHT, IM_WIDTH;;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  int option = 1;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if (sockfd < 0) error("ERROR opening socket");

  bzero((char*)&serv_addr, sizeof(serv_addr));
  portno = PORT;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0) error("ERROR on binding");

  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if (newsockfd < 0) error("ERROR on accept");

  int header[4];
  // bytes = recv(newsockfd, header, sizeof(int) * 4, 0);
  bool recv_ok = recv_all(newsockfd, header, sizeof(header));
  if (!recv_ok) error("ERROR on receiving header");
  int numImages = header[0];
  int imHeight = header[1];
  int imWidth = header[2];
  imgSize = header[3];
  printf("numImages: %d, imHeight: %d, imWidth: %d, imgSize: %d\n", numImages, imHeight, imWidth, imgSize);
  std::vector<uchar*> sockData;
  for (int i = 0; i < numImages; i++)
  {
    uchar sockDataTmp[imgSize];
    bool recv_ok = recv_all(newsockfd, sockDataTmp, sizeof(sockDataTmp));
    if (!recv_ok) error("ERROR on receiving image");
    // bytes = recv(newsockfd, sockDataTmp, imgSize, MSG_WAITALL);
    // if (bytes == -1) error("recv failed");

    // int start = 0;
    // while (true)
    // {
    //   bytes = recv(newsockfd, sockDataTmp + start, imgSize - start, 0);
    //   start += bytes;
    //   if (start == imgSize || bytes == 0) break;
    //   if (bytes == -1) error("recv failed");
    // }
    puts("sending feedback");
    double test_feedback[3] = {1.2, 3.4, 5.2};
    // n = send(newsockfd, test_feedback, sizeof(test_feedback), 0);
    // if (n < 0) error("ERROR writing to socket");
    bool send_ok = send_all(newsockfd, test_feedback, sizeof(test_feedback));
    if (!send_ok) error("ERROR sending feedback");
    puts("Data Send");    
    sockData.push_back(sockDataTmp);

  }
  printf("Total sockData size:%d\n", sockData.size());

  cv::Mat img(cv::Size(imWidth, imHeight), CV_8UC3, sockData[0]);
  // cv::Mat img(cv::Size(imWidth, imHeight), CV_8UC1, sockData[0]);
  double min, max;
  cv::Point minLoc;
  cv::Point maxLoc;
  cv::minMaxLoc(img.reshape(1), &min, &max, &minLoc, &maxLoc );
  std::cout << "Min: " << min << " Max: " << max << std::endl;
  // int ptr = 0;
  // for (int i = 0; i < img.rows; ++i)
  //   for (int j = 0; j < img.cols; ++j)
  //   {
  //     img.at<Vec3b>(i, j) = Vec3b(sockData[1][ptr + 0], sockData[1][ptr + 1], sockData[1][ptr + 2]);
  //     ptr = ptr + 3;
  //   }

  cv::namedWindow( "Server", cv::WINDOW_AUTOSIZE );// Create a window for display.
  cv::imshow( "Server", img );
  cv::imwrite("server.jpg", img);
  char key = cv::waitKey(0);

  close(newsockfd);
  close(sockfd);

  return 0;
}
