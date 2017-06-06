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

    nbytes=conn.recv(struct.calcsize("i") * 4)
    value = struct.unpack("i" * 4, nbytes)
    numImages, imWidth, imHeight, imgSize = (int(i) for i in value)
    print "numImage: %d, imHeight: %d, imWidth: %d, imgSize: %d" % (numImages, imHeight, imWidth, imgSize)
    imgs = []

    for idx in xrange(numImages):
        img=conn.recv(imgSize, socket.MSG_WAITALL)
        if img:
           imgs.append(img)
    test_cls = "apple"
    test_pos = np.array([1.2,3.4,5.2], dtype=np.float64) 
    num = np.array([numImages], dtype=np.int32) 
    conn.sendall(num.tostring())      
    for idx in xrange(numImages):
        nameLen = np.array(len(test_cls), dtype=np.int32)
        conn.sendall(nameLen.tostring())
        conn.sendall(test_cls)
        conn.sendall(test_pos.tostring())
    shape = (imHeight, imWidth, imgSize / imWidth / imHeight)
    cameraFeed = np.zeros(shape, np.uint8)
    cameraFeed = socketToNumpy(cameraFeed, imgs[0])

    conn.close()
    cv2.namedWindow("server");
    cv2.imshow("server", cameraFeed)
    key = cv2.waitKey(5000)
    
