from __future__ import print_function

import socket
import sys

import numpy as np
import struct

class Server:
    def __init__(self, *args, **kwargs):
        self.host = kwargs.get('host', 'None')
        self.port = kwargs.get('port', 7200)
        cvtypes = ['CV_8U', 'CV_8S', 'CV_16U', 'CV_16S', 'CV_32S', 'CV_32F', 'CV_64F']
        nptypes = [np.uint8, np.int8, np.uint16, np.int16, np.int32, np.float32, np.float64]
        self.int_to_cvtype = dict(zip(range(len(cvtypes)), cvtypes))
        self.int_to_nptype = dict(zip(range(len(nptypes)), nptypes))
        self.setup_connect_server()

    def setup_connect_server(self):
        for res in socket.getaddrinfo(self.host, self.port, socket.AF_UNSPEC,
                                      socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
            af, socktype, proto, canonname, sa = res
            try:
                s = socket.socket(af, socktype, proto)
                s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
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
        imgs= self.get_imgmat()
        return imgs

        
    def get_imgheader(self):
        nbytes = self.conn.recv(struct.calcsize("i") * 6, socket.MSG_WAITALL)
        value = struct.unpack("i" * 6, nbytes)
        self.numImages, self.imWidth, self.imHeight, self.imChannels, \
        self.imgSize, self.imType = (int(i) for i in value)
        print("numImage: %d, imHeight: %d, imWidth: %d, "
              "imChannels: %d, imgSize: %d, imType: %s" %\
              (self.numImages, self.imHeight, self.imWidth,
               self.imChannels, self.imgSize, self.int_to_cvtype[self.imType]))

    def get_imgmat(self):
        imgs = []
        for idx in xrange(self.numImages):
            img = self.conn.recv(self.imgSize, socket.MSG_WAITALL)
            if img:
                img = self.decode_image(img)
                imgs.append(img)
            'Color image and Depth image should have same height and width'
        return imgs

    def send_seg_result(self, cls_pos):
        num_objs = np.array([len(cls_pos)], dtype=np.int32)
        self.conn.sendall(num_objs.tostring())
        for cls, pos in cls_pos.iteritems():
            name_len = np.array(len(cls), dtype=np.int32)
            self.conn.sendall(name_len.tostring())
            self.conn.sendall(cls)
            self.conn.sendall(pos.astype(np.float64).tostring())

    def decode_image(self, sock_data):
        sock_data = np.fromstring(sock_data, self.int_to_nptype[self.imType])
        image = np.tile(sock_data, 1).reshape((self.imHeight,
                                               self.imWidth,
                                               self.imChannels))
        return image

    


