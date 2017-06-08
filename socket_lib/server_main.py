from __future__ import print_function
import socket
import sys

import cv2
import numpy as np
import ctypes
import struct
from Communication import Server

if __name__ == "__main__":
    kwargs = {'host': 'localhost',
              'port': 7200}
    server = Server(**kwargs)
    imgs = server.get_images()
    print('Min: ', np.min(imgs[0]), ' Max: ', np.max(imgs[0]))
    response = {'apple': np.random.uniform(0, 2, 3),
                'banana': np.random.uniform(0, 2, 3),
                'orange': np.random.uniform(0, 2, 3)}
    server.send_seg_result(response)
    for key, value in response.iteritems():
        print(key, ': ', value)
