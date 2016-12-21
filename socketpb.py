import socket
import time
import subprocess
from dummypic import upload

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
    f = open('uuid.txt', 'w')
    f.write(uuid)
    f.close()

    s.send("OK")
    s.close()
    print "Taking picture"
    return_code = subprocess.call("./picture.sh", shell=True)
    time.sleep(2)
