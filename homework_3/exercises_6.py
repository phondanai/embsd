import sys

from PIL import Image


if len(sys.argv) < 2:
    print("Please input filename to flip: ")
    print("usage: {} {}".format(sys.argv[0], "filename"))
    sys.exit(1)

filename = sys.argv[1]

im = Image.open(filename)
im.show(title="ORIGINAL")
#quantize = im.quantize(method=0, dither="NONE") # 0 means median cut
#quantize.show()

MEDIAN_CUT = 0

#for i in range(9):
#    quantize = im.quantize(colors=2**i, method=MEDIAN_CUT)
#    quantize.show()

quantize = im.quantize(method=MEDIAN_CUT, dither="NONE") # 0 means median cut
quantize.show()
#quantize.convert('RGB').save('quantize.jpeg')


