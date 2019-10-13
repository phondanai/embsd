import sys

from PIL import Image


if len(sys.argv) < 2:
    print("Please input filename")
    sys.exit(1)

filename = sys.argv[1]

im = Image.open(filename)

rotated = im.rotate(90)

rotated.show()
