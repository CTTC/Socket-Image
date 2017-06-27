import socket
import sys

import cv2
import numpy as np
import ctypes
import struct

HOST = None               # Symbolic name meaning all available interfaces
PORT = 7200              # Arbitrary non-privileged port

def socketToNumpy(cameraFeed, sockData):
    k=cameraFeed.shape[2]
    j=cameraFeed.shape[1]
    i=cameraFeed.shape[0]
    sockData = np.fromstring(sockData, np.uint8)
    cameraFeed = np.tile(sockData, 1).reshape((i,j,k))
    return cameraFeed

if __name__ == "__main__":
    for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC,
                                  socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
        af, socktype, proto, canonname, sa = res
        try:
            s = socket.socket(af, socktype, proto)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        except socket.error as msg:
            s = None
            continue
        try:
            s.bind(sa)
            s.listen(1)
        except socket.error as msg:
            s.close()
            s = None
            continue
        break
    if s is None:
        print 'could not open socket'
        sys.exit(1)
    conn, addr = s.accept()
    print 'Connected by', addr

    nbytes=conn.recv(struct.calcsize("i") * 4, socket.MSG_WAITALL)
    value = struct.unpack("i" * 4, nbytes)
    numImages, imHeight, imWidth, imgSize = (int(i) for i in value)
    print "numImage: %d, imHeight: %d, imWidth: %d, imgSize: %d" % (numImages, imHeight, imWidth, imgSize)
    imgs = []
    test_feedback = np.array([1.2,3.4,5.2], dtype=np.float64)
    for idx in xrange(numImages):
        img=conn.recv(imgSize, socket.MSG_WAITALL)
        if img:
           imgs.append(img)
           conn.sendall(test_feedback.tostring())
    shape = (imHeight, imWidth, imgSize / imWidth / imHeight)
    cameraFeed = np.zeros(shape, np.uint8)
    cameraFeed = socketToNumpy(cameraFeed, imgs[0])
    # sockData = b''
    # while imgSize:
    #     nbytes=conn.recv(imgSize)
    #     if not nbytes: break; result = False
    #     sockData+=nbytes
    #     imgSize-=len(nbytes)

    conn.close()
    cv2.namedWindow("server");
    cv2.imshow("server", cameraFeed)
    key = cv2.waitKey(5000)
    
