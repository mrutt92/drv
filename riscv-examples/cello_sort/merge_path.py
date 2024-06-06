import numpy as np

v0 = np.array([2, 3, 17, 29, 35, 73, 86, 90, 95, 99])
v1 = np.array([3, 5, 12, 22, 45, 64, 69, 82])
#v0 = np.array([17, 29, 35, 73])
#v1 = np.array([0, 100])

merge_matrix = np.greater.outer(v0, v1)
print("Merge Matrix:\n{}".format(merge_matrix))

def find_merge_path_intersection_linear(v0, v1, k):
    # linear search along the k-th diagonal
    i = min(k, len(v0))
    j = k - i
    while i > 0 and j < len(v1) and v0[i-1] > v1[j]:
        i -= 1
        j += 1
    return i, j


def first_on_diagonal(v0, v1, k):
    i_start = min(k, len(v0)-1)
    j_start = k - i_start
    return i_start, j_start

# return the n-th index on the k-th diagonal
def nth_on_diagonal(v0, v1, n, k):
    i,j = first_on_diagonal(v0,v1,k)
    return i-n, j+n

def diagonal_search_space_size(v0, v1, k):
    i,j = first_on_diagonal(v0,v1,k)
    return min(i, len(v1)-j)

# def diagonal_search_space(v0, v1, k):
#     i,j = first_on_diagonal(v0,v1,k)
#     s = []
#     while i > 0 and j < len(v1):
#         s.append((i,j))        
#         i -= 1
#         j += 1
    
#     return s

# return the length of the diagonal
# def diagonal_length(v0, v1, k):    
#     i,j = first_on_diagonal(v0,v1,k)
#     return max(min(i+1, len(v1)-j),0)

def find_merge_path_intersection_binary(v0, v1, k):
    # binary search along the k-th diagonal
    n = diagonal_search_space_size(v0, v1, k)

    lower = 0
    upper = n
    hi = upper
    lo = lower
    while lo < hi:
        m = (lo+hi)//2
        i,j = nth_on_diagonal(v0, v1, m, k)
        if v0[i-1] > v1[j]:
            lo = m+1
        else:
            hi = m

    return nth_on_diagonal(v0, v1, lo, k)

def find_merge_path_intersection(v0, v1, k):
    i,j = find_merge_path_intersection_binary(v0, v1, k)
    print("Merge path intersection with {}-th diagonal at: {}".format(k, (i,j)))
    return i,j

def merge_range(v0, v1, vo, start, stop):
    i,j = find_merge_path_intersection(v0, v1, start)
    while i+j < stop and i < len(v0) and j < len(v1):
        if (v0[i] < v1[j]):
            vo[i+j] = v0[i]
            i += 1
        else:
            vo[i+j] = v1[j]        
            j += 1

    while i < len(v0) and i+j < stop:
        vo[i+j] = v0[i]
        i += 1

    while j < len(v1) and i+j < stop:
        vo[i+j] = v1[j]
        j += 1

vo = np.zeros(len(v0)+len(v1))

n = len(v0) + len(v1)
d = n//2

print("v0: {}".format(v0))
print("v1: {}".format(v1))

#print("diagonal search space: {}".format(diagonal_search_space(v0, v1, len(v0)+len(v1)-2)))

merge_range(v0, v1, vo, d, n)
print("vo: {}".format(vo))

merge_range(v0, v1, vo, 0, d)
print("vo: {}".format(vo))

# for diag in range(len(v0)+len(v1)-1):
#     #print("Merge path intersection with {}-th diagonal at: {}".format(diag, find_merge_path_intersection(v0, v1, diag)))
#     #print("First on diagonal {} is: {}".format(diag, first_on_diagonal(v0, v1, diag)))
#     #print("0th on diagonal {} is: {}".format(diag, nth_on_diagonal(v0, v1, 0, diag)))
#     print("Length of diagonal {} is: {}".format(diag, diagonal_length(v0, v1, diag)))
#     print("Binary: Merge path intersection with {}-th diagonal at: {}".format(diag, find_merge_path_intersection_binary(v0, v1, diag)))
#     print("Linear: Merge path intersection with {}-th diagonal at: {}".format(diag, find_merge_path_intersection_linear(v0, v1, diag)))    
    
