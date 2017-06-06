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

  int header[1];
  bytes = recv(newsockfd, header, sizeof(int), 0);
  int numImages = header[0];
  bytes = recv(newsockfd, header, sizeof(int), 0);
  int imHeight = header[0];
  bytes = recv(newsockfd, header, sizeof(int), 0);
  int imWidth = header[0];
  bytes = recv(newsockfd, header, sizeof(int), 0);
  imgSize = header[0];
  printf("numImages: %d, imHeight: %d, imWidth: %d, imgSize: %d\n", numImages, imHeight, imWidth, imgSize);
  std::vector<uchar*> sockData;
  for (int i = 0; i < numImages; i++)
  {
    uchar sockDataTmp[imgSize];
    int start = 0;
    // bytes = recv(newsockfd, sockDataTmp, imgSize, MSG_WAITALL);
    // if (bytes == -1) error("recv failed");
    while (true)
    {
      bytes = recv(newsockfd, sockDataTmp + start, imgSize - start, 0);
      start += bytes;
      if (start == imgSize || bytes == 0) break;
      if (bytes == -1) error("recv failed");
    }
    puts("sending feedback");
    char received_str[100] = "received";
    n = send(newsockfd, received_str, 100, 0);
    if (n < 0) error("ERROR writing to socket");
    sockData.push_back(sockDataTmp);

  }
  printf("Total sockData size:%d\n", sockData.size());

  cv::Mat img(cv::Size(imWidth, imHeight), CV_8UC3, sockData[0]);
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
