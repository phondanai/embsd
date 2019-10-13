import sys

input_number = sys.argv[1:]

def print_arr(arr):
    for i in arr:
        print("{} ".format(i), end="")

    print()

print("Input number:")
print_arr(input_number)

print("After sorted")
sorted_arr = []
sorted_number = sorted([int(i) for i in input_number])

print_arr(sorted_number)
