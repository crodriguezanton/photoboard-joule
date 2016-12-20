import requests

# POST request to server sending as file data both COLOR and DEPTH pictures
files = {'picture': open('/home/root/photoboard-image-COLOR.png', 'rb'),
        'depth': open('/home/root/photoboard-image-DEPTH.png', 'rb')}
requests.post('http://photoboard.tech/api/picture_server/', files=files)
