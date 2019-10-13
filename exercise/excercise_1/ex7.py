import sys

if len(sys.argv) < 2:
    print("please input at least 1 number")
    sys.exit(1)

inputs = [int(i) for i in sys.argv[1:]]

for i in sorted(inputs):
    print(i)
