import sys

from PIL import Image


if len(sys.argv) < 2:
    print("Please input filename to flip: ")
    print("usage: {} {}".format(sys.argv[0], "filename"))
    sys.exit(1)

filename = sys.argv[1]

im = Image.open(filename)
im.show(title="ORIGINAL")

MAXIMUM_COVERAGE = 1

quantize = im.quantize(method=MAXIMUM_COVERAGE, dither="NONE")
quantize.show()


