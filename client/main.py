import socket               # Import socket module
from os import path

s = socket.socket()         # Create a socket object
host = socket.gethostname() # Get local machine name
port = 4242                 # Reserve a port for your service.

s.connect((host, port))


print('Ready! Create /tmp/go to start!')

while not path.exists("/tmp/go"):
    pass

print('Sending...')
while True:
    # f = open('alice_in_wonderland.txt','r')
    # l = f.read(1024)
    # while (l):
    #     print('Sending...')
    #     s.send(l.encode('ascii'))
    #     l = f.read(1024)
    # f.close()
    s.send(b'Hello!')

print("Done Sending")
s.close   