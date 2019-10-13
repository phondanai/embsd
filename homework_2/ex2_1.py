import random

arr = []

# Random 999 numbers
for i in range(999):
    arr.append(random.randint(1,100000))

arr.sort()

print("Median: {}".format(arr[len(arr)//2]))
