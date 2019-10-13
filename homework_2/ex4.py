#def mergeSort(alist):
#    print("Splitting ",alist)
#    if len(alist)>1:
#        mid = len(alist)//2
#        lefthalf = alist[:mid]
#        righthalf = alist[mid:]
#
#        mergeSort(lefthalf)
#        mergeSort(righthalf)
#
#        i=0
#        j=0
#        k=0
#        while i < len(lefthalf) and j < len(righthalf):
#            if lefthalf[i] <= righthalf[j]:
#                alist[k]=lefthalf[i]
#                i=i+1
#            else:
#                alist[k]=righthalf[j]
#                j=j+1
#            k=k+1
#
#        while i < len(lefthalf):
#            alist[k]=lefthalf[i]
#            i=i+1
#            k=k+1
#
#        while j < len(righthalf):
#            alist[k]=righthalf[j]
#            j=j+1
#            k=k+1
#    print("Merging ",alist)
#
#alist = [54,26,93,17,77,31,44,55,20]
#mergeSort(alist)
#print(alist)

#def merge(S1, S2, S):
#    print("-"*80)
#    print("MERGE")
#    print("\t\tS:{}".format(S))
#    print("S1:",S1)
#    print("S2:".rjust(20),S2)
#    i = j = 0
#    while i+j < len(S):
#        if j == len(S2) or (i < len(S1) and S1[i] < S2[j]):
#            S[i+j] = S1[i]
#            i += 1
#        else:
#            S[i+j] = S2[j]
#            j += 1
#
#def merge_sort(S):
#    n = len(S)
#    if n < 2:
#        return
#    mid = n // 2
#    S1 = S[0:mid]
#    S2 = S[mid:n]
#
#    print("S: \t",S)
#    print("S1:",S1)
#    print("S2:\t\t",S2)
#
#    merge_sort(S1)
#    merge_sort(S2)
#
#    merge(S1, S2, S)
#
#arr = [20,30,1,2,4,90,22]
#
#merge_sort(arr)
#print(" ".join("{}".format(i) for i in arr))

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

