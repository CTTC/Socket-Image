#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <ctime>

#define LOCALHOST "localhost"
#define PORT 7200


void error(const char *msg)
{
  perror(msg);
  exit(0);
}

int main()
{
  int sockfd, portno, n, imgSize, IM_HEIGHT, IM_WIDTH;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  cv::Mat cameraFeed;

  portno = PORT;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("ERROR opening socket");

  server = gethostbyname(LOCALHOST);

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  cameraFeed = cv::imread("image.jpg", cv::IMREAD_COLOR );

  // cameraFeed = cv::imread("depth.png", cv::IMREAD_COLOR );
  if (!cameraFeed.data)
  {
    std::cout <<  "Could not open or find the image" << std::endl ;
    return -1;
  }
  int height = cameraFeed.rows;
  int width = cameraFeed.cols;

  imgSize = cameraFeed.total() * cameraFeed.elemSize();

  int numImages = 100;
  n = send(sockfd, &numImages, sizeof(int), 0);
  if (n < 0) error("ERROR writing to socket");
  n = send(sockfd, &height, sizeof(int), 0);
  if (n < 0) error("ERROR writing to socket");
  n = send(sockfd, &width, sizeof(int), 0);
  if (n < 0) error("ERROR writing to socket");
  n = send(sockfd, &imgSize, sizeof(int), 0);
  if (n < 0) error("ERROR writing to socket");
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  uchar received_str[100];
  for (int idx = 0; idx < numImages; idx++)
  {
    n = send(sockfd, cameraFeed.data, imgSize, 0);
    if (n < 0) error("ERROR writing to socket");
    puts("Data Send");
    int bytes = recv(sockfd, received_str, 100, 0);
    if (bytes == -1) error("recv failed");
    puts("  Feedback Received");
  }
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);
  std::cout << "finished computation at " << std::ctime(&end_time)
            << "elapsed time: " << elapsed_seconds.count() << "s\n";
  close(sockfd);
  return 0;
}
