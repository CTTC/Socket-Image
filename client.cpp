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
#include <arpa/inet.h> 
#define LOCALHOST "localhost"
#define PORT 7200


void error(const char *msg)
{
  perror(msg);
  exit(0);
}
bool recv_all(int socket, void *buffer, int length)
{
  uchar *ptr = (uchar*) buffer;
  int bytes = 0;
  while (length > 0)
  {
    puts("rece");
    bytes = recv(socket, ptr, length, 0);
    std::cout << "bytes " << bytes << "  length: " << length << std::endl;
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

int main(int argc, char *argv[])
{
  int sockfd, portno, n, imgSize, IM_HEIGHT, IM_WIDTH;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  cv::Mat cameraFeed;

  portno = PORT;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) error("ERROR opening socket");
  // // If you want to connect to the localhost
  // server = gethostbyname(LOCALHOST);

  // if (server == NULL) {
  //   fprintf(stderr, "ERROR, no such host\n");
  //   exit(0);
  // }

  // bzero((char *) &serv_addr, sizeof(serv_addr));
  // serv_addr.sin_family = AF_INET;
  // bcopy((char *)server->h_addr,
  //       (char *)&serv_addr.sin_addr.s_addr,
  //       server->h_length);
  // serv_addr.sin_port = htons(portno);

  // If you want to connect to a machine by its ip address
  if (argc != 2)
  {
    printf("\n Usage: %s <ip of server> \n", argv[0]);
    return 1;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);

  if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
  {
    printf("\n inet_pton error occured\n");
    return 1;
  }
  if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");

  cameraFeed = cv::imread("image.jpg", cv::IMREAD_COLOR);

  // cameraFeed = cv::imread("depth.png", cv::IMREAD_ANYDEPTH);
  std::cout << "channel: " << cameraFeed.channels() << "  " << cameraFeed.rows << "  " << cameraFeed.cols << std::endl;
  if (!cameraFeed.data)
  {
    std::cout <<  "Could not open or find the image" << std::endl ;
    return -1;
  }
  int height = cameraFeed.rows;
  int width = cameraFeed.cols;

  imgSize = cameraFeed.total() * cameraFeed.elemSize();

  double min, max;
  cv::Point minLoc;
  cv::Point maxLoc;
  cameraFeed = cameraFeed.reshape(1);
  cv::minMaxLoc(cameraFeed, &min, &max, &minLoc, &maxLoc );
  std::cout << "Min: " << min << " Max: " << max << std::endl;
  std::cout << "image size: " << imgSize << std::endl;
  int numImages = 10;
  std::vector<int> header = {numImages, height, width, imgSize};
  for (int i = 0; i < header.size(); i++)
  {
    bool send_ok = send_all(sockfd, &header[i], sizeof(int));
    if (!send_ok) error("ERROR sending header");
  }
  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();

  double received_str[3] = {0.0};
  for (int idx = 0; idx < numImages; idx++)
  {
    // n = send(sockfd, cameraFeed.data, imgSize, 0);
    bool send_ok = send_all(sockfd, cameraFeed.data, imgSize);
    if (!send_ok) error("ERROR sending Image");
    puts("Data Send");
    bool rev_ok = recv_all(sockfd, received_str, sizeof(received_str));
    if (!rev_ok) error("ERROR receiving feedback");
    // int bytes = recv(sockfd, received_str, sizeof(received_str), 0);
    // if (bytes == -1) error("recv failed");
    std::cout << " Testing Feedback Received: " << received_str[0] << "  " <<
              received_str[1] << "  " << received_str[2] << std::endl;

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
