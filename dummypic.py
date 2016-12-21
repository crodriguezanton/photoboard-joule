import requests

def upload():
    f = open('uuid.txt', 'r')
    uuid = f.readline()
    print 'http://photoboard.tech/api/upload-picture/'+uuid+'/'
    f.close()
    files = {'picture': open('/home/root/photoboard-image-COLOR.png', 'rb'),
            'depth': open('/home/root/photoboard-image-DEPTH.png', 'rb')}
    r = requests.post('http://photoboard.tech/api/upload-picture/'+uuid+'/',files=files)
    print r
    print r.text
