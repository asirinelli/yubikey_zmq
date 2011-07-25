#!/usr/bin/env python
import sys
import zmq

context = zmq.Context()
socket = context.socket(zmq.REQ)
socket.connect(sys.argv[1])
print sys.argv[1]

req = ' '.join(sys.argv[2:])
print req
socket.send(req)
rep = socket.recv()
print "Received:", rep 
