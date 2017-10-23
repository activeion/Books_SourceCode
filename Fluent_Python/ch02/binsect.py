import bisect
import sys

HAYSTACK = [1, 4, 5, 6, 8, 12, 15, 20, 21, 23, 23, 26, 29, 30]
NEEDLES = [0, 1, 2, 5, 8, 10, 22, 23, 29, 30, 31]

ROW_FMT = '{0:2d} @ {1:2d}    {2}{0:3d}'


def halfsearch(ar, target):
    lo=0
    hi=len(ar)-1 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if(target<ar[mid]): #升序,左半边
            hi=mid-1#出界点
        elif (target>ar[mid]): #升序,右半边
            lo=mid+1#入界点
        else:# target == ar[mid]
            #print("just found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
            return mid+1
    #print("nothing found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
    if(target>=ar[-1]):
        return len(ar)
    elif(target<=ar[0]):
        return 0
    else:
        return mid+1

def halfsearch_left(ar, target):
    lo=0
    hi=len(ar)-1 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if(target<ar[mid]): #升序,左半边
            hi=mid-1#出界点
        elif (target>ar[mid]): #升序,右半边
            lo=mid+1#入界点
        else:# target == ar[mid]
            #print("just found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
            return mid
    #print("nothing found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
    if(target>ar[-1]):
        return len(ar)
    elif(target<=ar[0]):
        return 0
    else:
        return mid+1

def halfsearch_d(ar, target):
    lo=0
    hi=len(ar)-1 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if(target>ar[mid]): #升序,左半边
            hi=mid-1#出界点
        elif (target<ar[mid]): #升序,右半边
            lo=mid+1#入界点
        else:# target == ar[mid]
            #print("just found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
            return mid+1
    #print("nothing found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
    if(target>=ar[0]):
        return 0
    elif(target<=ar[-1]):
        return len(ar)
    else:
        return mid+1

def demo(bisect_fn):
    for needle in reversed(NEEDLES):
        position = bisect_fn(HAYSTACK, needle)
        offset = position*"  |"
        print(ROW_FMT.format(needle, position, offset))

def grade(bisect_fn, score, breakpoints =[60, 70, 80, 90], grades='FDCBA'):
    i = bisect_fn(breakpoints, score)
    return grades[i]

if __name__ == '__main__':
    if sys.argv[-1] == 'left':
        bisect_fn = bisect.bisect_left
    elif sys.argv[-1] == 'halfsearch_left':
        bisect_fn = halfsearch_left
    elif sys.argv[-1] == 'halfsearch':
        bisect_fn = halfsearch
    elif sys.argv[-1] == 'halfsearch_d':
        HAYSTACK =[x for x in reversed(HAYSTACK)]
        bisect_fn = halfsearch_d
    else:
        bisect_fn = bisect.bisect

    print('DEMO:', bisect_fn.__name__)
    print('index    ->', ' '.join('%2d' % n for n in range(len(HAYSTACK)+1)))
    print('haystack ->', ' '.join('%2d' % n for n in HAYSTACK))
    demo(bisect_fn)


    scores = [33, 99, 77, 70, 89, 90, 100, 101, -1, 60]
    print(scores)
    print([grade(bisect_fn,score) for score in scores]) 





