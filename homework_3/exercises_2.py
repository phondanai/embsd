import sys

from PIL import Image


if len(sys.argv) < 2:
    print("Please input filename to flip: ")
    print("usage: {} {}".format(sys.argv[0], "filename"))
    sys.exit(1)

filename = sys.argv[1]

im = Image.open(filename)

flip_upside_down = im.transpose(method=Image.FLIP_TOP_BOTTOM)

flip_upside_down.show()
