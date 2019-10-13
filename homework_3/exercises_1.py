import sys

from PIL import Image, ImageOps


def negative(im):
    px = greyscale.load()
    for i in range(greyscale.size[0]):
        for j in  range(greyscale.size[1]):
            px[i,j] = 255 - px[i,j]

if len(sys.argv) < 2:
    print("Please input filename to flip: ")
    print("usage: {} {}".format(sys.argv[0], "filename"))
    sys.exit(1)

filename = sys.argv[1]

greyscale = Image.open(filename).convert("L")

neg = ImageOps.invert(greyscale)
neg.show()

negative(greyscale)

greyscale.show()
