"""
A "mirroring" ``stdout`` context.
"""

# BEGIN MIRROR_EX
class LookingGlass:

    def __enter__(self):  # <1>
        import sys
        self.original_write = sys.stdout.write  # <2>
        sys.stdout.write = self.reverse_write  # <3> 偷天换日，实现反向输出
        return 'JABBERWOCKY'  # <4>

    def reverse_write(self, text):  # <5>
        self.original_write(text[::-1])

    def __exit__(self, exc_type, exc_value, traceback):  # <6>
        import sys  # <7>
        sys.stdout.write = self.original_write  # <8>
        if exc_type is ZeroDivisionError:  # <9>
            print('Please DO NOT divide by zero!')
            return True  # <10>
        # <11>
# END MIRROR_EX

if __name__ == '__main__':
    with open('mirror.py') as fp: # 打开自己的源代码文件
        src = fp.read(60)
    print(len(src)) # 60
    print(fp)
    print(fp.closed, fp.encoding)
    try:
        fp.read(60)
    except ValueError:
        print('ValueError, fp can not be used again')

    # While active, the context manager reverses text output to ``stdout``::
    # BEGIN MIRROR_DEMO_1
    print('----------------------------')
    with LookingGlass() as what:  # <1>
        print('Alice, Kitty and Snowdrop')  # <2> pordwonS dna yttiK ,ecilA  # <3>
        print(what) # YKCOWREBBAJ
    print(what) # <4> 'JABBERWOCKY'
    print('Back to normal.')  # <5> Back to normal.
    # END MIRROR_DEMO_1


    # This exposes the context manager operation::
    # BEGIN MIRROR_DEMO_2
    print('----------------------------')
    manager = LookingGlass()  # <1> 开始反向输出
    print(manager) # <mirror.LookingGlass object at 0x2a578ac>
    monster = manager.__enter__()  # <2>
    print(monster == 'JABBERWOCKY') # <3> eurT 反向输出
    print(monster) # 'YKCOWREBBAJ' 反向输出
    print(manager) # >ca875a2x0 ta tcejbo ssalGgnikooL.rorrim< 反向输出
    manager.__exit__(None, None, None)  # <4> 恢复为正向
    print(monster) # 'JABBERWOCKY' 正向输出
    # END MIRROR_DEMO_2

    # The context manager can handle and "swallow" exceptions.
    # BEGIN MIRROR_DEMO_3
    print('----------------------------')
    with LookingGlass():
        print('Humpty Dumpty')
        x = 1/0  # <1> Please DO NOT divide by zero!
        print('END')  # <2>
    print('----------------------------')
    with LookingGlass():
        print('Humpty Dumpty')
        #x = no_such_name  # <1>
        #print('END')  # <2> ...  Traceback (most recent call last): ...  NameError: name 'no_such_name' is not defined
    # END MIRROR_DEMO_3
