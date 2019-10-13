import sys

if len(sys.argv) < 2:
    print("usage: {} text you want to sort".format(sys.argv[0]))
    sys.exit(0)

print(" ".join(sorted(sys.argv[1:])))

