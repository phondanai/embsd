def merge_sort(aList):
    if len(aList) <= 1:
        return aList

    middle = len(aList) // 2

    left = merge_sort(aList[:middle])
    right = merge_sort(aList[middle:])
    return merge(left, right)

def merge(left, right):
    result = []
    while(left and right):
        if left[0] > right[0]:
            result.append(right[0])
            right.pop(0)
        else:
            result.append(left[0])
            left.pop(0)

    while(left):
        result.append(left[0])
        left.pop(0)
    while(right):
        result.append(right[0])
        right.pop(0)

    return result

test_list = [2,10,3,100,9,99,23,42,333]
print("Before sort: {}".format(test_list))

print("After sort: {}".format(merge_sort(test_list)))

