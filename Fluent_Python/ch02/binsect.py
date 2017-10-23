import bisect
import sys

HAYSTACK = [1, 4, 5, 6, 8, 12, 15, 20, 21, 23, 23, 26, 29, 30]
NEEDLES = [0, 1, 2, 5, 8, 10, 22, 23, 29, 30, 31]

ROW_FMT = '{0:2d} @ {1:2d}    {2}{0:3d}'

scores = [33, 99, 77, 70, 89, 90, 100, 101, -1, 60]
breakpoints = [60, 70, 80, 90]

def halfsearch(ar, target):
    lo=0
    hi=len(ar) 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if(target<ar[mid]): #升序,左半边
            hi=mid          #调整出界点
        else:               #升序,右半边
            #print("just found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
            lo=mid+1
    return lo

def halfsearch_left(ar, target):
    lo=0
    hi=len(ar) 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if (target>ar[mid]):#升序,右半边
            lo=mid+1        #调整入界点
        else:               #升序,左半边
            hi=mid
            #print("just found. lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
    return lo

def halfsearch_d(ar, target):
    lo=0
    hi=len(ar) 
    mid=0
    while(lo<hi):
        mid = (hi+lo)//2
        #print("processing..., lo=%d,mid=%d,hi=%d,ar[mid]=%d\n" %(lo, mid, hi,ar[mid]))
        if(target>ar[mid]): #升序,左半边
            hi=mid#出界点
        else: #升序,右半边
            lo=mid+1#入界点
    return lo

def demo(bisect_fn):
    for needle in reversed(NEEDLES):
        position = bisect_fn(HAYSTACK, needle)
        offset = position*"  |"
        print(ROW_FMT.format(needle, position, offset))

def grade(bisect_fn, score, breakpoints , grades='FDCBA'):
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
        breakpoints = [x for x in reversed(breakpoints)]
        bisect_fn = halfsearch_d
    else:
        bisect_fn = bisect.bisect

    print('DEMO:', bisect_fn.__name__)
    print('index    ->', ' '.join('%2d' % n for n in range(len(HAYSTACK)+1)))
    print('haystack ->', ' '.join('%2d' % n for n in HAYSTACK))
    demo(bisect_fn)

    print(scores)
    print([grade(bisect_fn,score,breakpoints) for score in scores]) 





