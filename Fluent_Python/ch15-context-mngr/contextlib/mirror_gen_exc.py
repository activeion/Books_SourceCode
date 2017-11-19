"""
A "mirroring" ``stdout`` context manager.

comined mirror_gen.py & mirror_gen_exc.py
"""

# BEGIN MIRROR_GEN_EXC
import contextlib

@contextlib.contextmanager
def looking_glass():
    # __enter__() defination
    import sys
    original_write = sys.stdout.write

    def reverse_write(text):
        original_write(text[::-1])

    sys.stdout.write = reverse_write

    msg = ''  # <1>
    try:
        yield 'JABBERWOCKY'

    # __exit__() defination
    except ZeroDivisionError:  # <2>
        msg = 'Please DO NOT divide by zero!'
    finally:
        sys.stdout.write = original_write  # <3>
        if msg:
            print(msg)  # <4>
# END MIRROR_GEN_EXC

'''
The context manager can handle and "swallow" exceptions.
The following test does not pass under doctest (a
ZeroDivisionError is reported by doctest) but passes
if executed by hand in the Python 3 console (the exception
is handled by the context manager):
'''

if __name__ == '__main__':
    # While active, the context manager reverses text output to ``stdout``::
    # BEGIN MIRROR_GEN_DEMO_1
    with looking_glass() as what:  # <1>
        print('Alice, Kitty and Snowdrop')  # pordwonS dna yttiK ,ecilA
        print(what) # YKCOWREBBAJ
    print(what) # 'JABBERWOCKY'
    # END MIRROR_GEN_DEMO_1

    # This exposes the context manager operation::
    # BEGIN MIRROR_GEN_DEMO_2
    manager = looking_glass()  # <1>
    print(manager) # doctest: +ELLIPSIS <contextlib._GeneratorContextManager object at 0x...>
    monster = manager.__enter__()  # <2>
    print(monster == 'JABBERWOCKY')  # <3> eurT
    print(monster) # 'YKCOWREBBAJ'
    print(manager) # doctest: +ELLIPSIS >...x0 ta tcejbo reganaMtxetnoCrotareneG_.biltxetnoc<
    manager.__exit__(None, None, None)  # <4>
    print(monster) # 'JABBERWOCKY'
    # END MIRROR_GEN_DEMO_2
    
    # except test
    # BEGIN MIRROR_GEN_DEMO_3
    with looking_glass():
        print('Humpty Dumpty') # ytpmuD ytpmuH
        x = 1/0  # <1> Please DO NOT divide by zero! 正常顺序打印
        print('END')  # <2> 得不到执行的机会
    # END MIRROR_GEN_DEMO_3

    with looking_glass():
        print('Humpty Dumpty')
        x = no_such_name  # <1> Traceback (most recent call last): ... NameError: name 'no_such_name' is not defined
        print('END')  # <2>
