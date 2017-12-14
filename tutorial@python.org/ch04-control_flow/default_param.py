def f(a, L=[]):
    L.append(a)
    return L

def main():
    print(f(1))
    print(f(2))
    print(f(3))

if __name__ == '__main__':
    main()
    print(f(4))

"""
[1]
[1, 2]
[1, 2, 3]
[1, 2, 3, 4]
"""
