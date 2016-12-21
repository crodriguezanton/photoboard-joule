import sys
from PIL import Image

images = map(Image.open, ['photoboard-image-COLOR1.png', 'photoboard-image-COLOR2.png'])
widths, heights = zip(*(i.size for i in images))

total_width = max(widths)
max_height = max(heights)

new_im = Image.new('RGB', (total_width, max_height))
print "Combining Images"
left = True
for im in images:
    w, h = im.size
    if left:
        new_im.paste(im.crop((0, 0, w/2, h)), (0,0))
    else:
        new_im.paste(im.crop((w/2, 0, w, h)), (w/2,0))
    left=False

new_im.save('photoboard-image-COLOR.png')
print "Saved"
