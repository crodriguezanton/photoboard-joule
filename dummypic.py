import requests

def upload():
    f = open('uuid.txt', 'r')
    files = {'picture': open('/home/root/photoboard-image-COLOR.png', 'rb'),
            'depth': open('/home/root/photoboard-image-DEPTH.png', 'rb')}
    requests.post('http://photoboard.tech/api/upload-picture/'+f.readline(),files=files)
