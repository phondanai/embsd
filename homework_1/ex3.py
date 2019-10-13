import sys

up_to = int(sys.argv[1])

div = 2

result = []

while(up_to != 1):
    if(up_to % div != 0):
        div = div + 1
    else:
        up_to = up_to / div
        result.append(str(div))
        #print(div, end=" * ")

print("{} = ".format(sys.argv[1]), end="")
print(" *".join(result))
