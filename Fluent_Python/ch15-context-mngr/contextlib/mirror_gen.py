"""
A "mirroring" ``stdout`` context manager.
"""

# BEGIN MIRROR_GEN_EX
import contextlib

@contextlib.contextmanager  # <1>
def looking_glass():
    import sys
    original_write = sys.stdout.write  # <2>

    def reverse_write(text):  # <3>
        original_write(text[::-1])

    sys.stdout.write = reverse_write  # <4>
    yield 'JABBERWOCKY'  # <5>
    sys.stdout.write = original_write  # <6>
# END MIRROR_GEN_EX


if __name__ == '__main__':
    # While active, the context manager reverses text output to ``stdout``::
    # BEGIN MIRROR_GEN_DEMO_1
    from mirror_gen import looking_glass
    with looking_glass() as what:  # <1>
        print('Alice, Kitty and Snowdrop')  # pordwonS dna yttiK ,ecilA
        print(what) # YKCOWREBBAJ
    print(what) # 'JABBERWOCKY'
    # END MIRROR_GEN_DEMO_1


    # This exposes the context manager operation::
    # BEGIN MIRROR_GEN_DEMO_2
    from mirror_gen import looking_glass
    manager = looking_glass()  # <1>
    print(manager) # doctest: +ELLIPSIS <contextlib._GeneratorContextManager object at 0x...>
    monster = manager.__enter__()  # <2>
    print(monster == 'JABBERWOCKY')  # <3> eurT
    print(monster) # 'YKCOWREBBAJ'
    print(manager) # doctest: +ELLIPSIS >...x0 ta tcejbo reganaMtxetnoCrotareneG_.biltxetnoc<
    manager.__exit__(None, None, None)  # <4>
    print(monster) # 'JABBERWOCKY'
    # END MIRROR_GEN_DEMO_2

