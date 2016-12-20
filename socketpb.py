import socket
import time

while True:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print "Socket created"
    try:
        print "Connecting"
        s.connect(("photoboard.tech", 8008))
    except:
        print "Except"
        time.sleep(2)
        continue

    s.send("CONN")
    uuid = s.recv(40)
    print uuid

    s.send("OK")
    s.close()

    time.sleep(2)
