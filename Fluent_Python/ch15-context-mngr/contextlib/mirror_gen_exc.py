"""
A "mirroring" ``stdout`` context manager.
"""

# BEGIN MIRROR_GEN_EXC
import contextlib

@contextlib.contextmanager
def looking_glass():
    import sys
    original_write = sys.stdout.write

    def reverse_write(text):
        original_write(text[::-1])

    sys.stdout.write = reverse_write
    msg = ''  # <1>
    try:
        yield 'JABBERWOCKY'
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
    # BEGIN MIRROR_GEN_DEMO_3
    from mirror_gen import looking_glass
    with looking_glass():
        print('Humpty Dumpty') # ytpmuD ytpmuH
        x = 1/0  # <1> Please DO NOT divide by zero!
        print('END')  # <2>
    # END MIRROR_GEN_DEMO_3

    with looking_glass():
        print('Humpty Dumpty')
        x = no_such_name  # <1> Traceback (most recent call last): ... NameError: name 'no_such_name' is not defined
        print('END')  # <2>
