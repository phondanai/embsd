import random

arr = []

# Random 9 numbers each range from 1-100
for i in range(9):
    arr.append(random.randint(1,100))

print("Before sorted: {}".format(arr))
arr.sort()
print("After sorted: {}".format(arr))

print("Median: {}".format(arr[len(arr)//2]))
