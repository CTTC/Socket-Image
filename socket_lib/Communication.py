from __future__ import print_function

import socket
import sys

import numpy as np
import struct

class Server:
    def __init__(self, *args, **kwargs):
        self.host = kwargs.get('host', 'None')
        self.port = kwargs.get('port', 7200)
        self.setup_connect_server()

    def setup_connect_server(self):
        for res in socket.getaddrinfo(self.host, self.port, socket.AF_UNSPEC,
                                      socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
            except socket.error as msg:
                raise msg
            try:
                s.bind(sa)
                s.listen(1)
            except socket.error as msg:
                s.close()
                raise msg
            break
        self.conn, addr = s.accept()
        print('Server Connected by', addr)

    def get_images(self):
        self.get_imgheader()
        color_img, depth_img = self.get_imgmat()
        return color_img, depth_img

        
    def get_imgheader(self):
        nbytes = self.conn.recv(struct.calcsize("i") * 4, socket.MSG_WAITALL)
        value = struct.unpack("i" * 4, nbytes)
        self.numImages, self.imWidth, self.imHeight, self.imgSize = (int(i) for i in value)
        print("numImage: %d, imHeight: %d, imWidth: %d, imgSize: %d" %\
              (self.numImages, self.imHeight, self.imWidth, self.imgSize))

    def get_imgmat(self):
        imgs = []
        for idx in xrange(self.numImages):
            img = self.conn.recv(self.imgSize, socket.MSG_WAITALL)
            if img:
                imgs.append(img)
        if len(imgs) > 2:
            print("!!! Warning: Only two images (Color Image and Depth Image) needed at a time")
            print("Taking the first image as the color image ...")
            print("Taking the second image as the depth image ...")
        color_img = self.decode_image(imgs[0])
        depth_img = self.decode_image(imgs[1])
        assert color_img.shape[2] == 3, 'Color image should have 3 channels'
        assert color_img.shape[2] == 1, 'Depth image should have 1 channel'
        assert color_img.shape[:2] == depth_img.shape, \
            'Color image and Depth image should have same height and width'
        return color_img, depth_img

    def send_seg_result(self, cls_pos):
        num_objs = np.array([len(cls_pos)], dtype=np.int32)
        self.conn.sendall(num_objs.tostring())
        for cls, pos in cls_pos.iteritems():
            name_len = np.array(len(cls), dtype=np.int32)
            self.conn.sendall(name_len.tostring())
            self.conn.sendall(cls)
            self.conn.sendall(pos.astype(np.float64).tostring())

    def decode_image(self, sock_data):
        sock_data = np.fromstring(sock_data, np.uint8)
        image = np.tile(sock_data, 1).reshape((self.imHeight,
                                               self.imWidth,
                                               sock_data.size / self.imHeight / self.imWidth))
        return image

    


