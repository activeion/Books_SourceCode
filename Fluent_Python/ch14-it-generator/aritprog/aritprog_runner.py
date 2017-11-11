import doctest
import importlib
import glob


TARGET_GLOB = 'class/aritprog*.py'


def main(argv):
    verbose = '-v' in argv
    for module_file_name in sorted(glob.glob(TARGET_GLOB)):
        module_name = module_file_name.replace('.py', '')
        module_name = module_name.replace('/', '.')
        module = importlib.import_module(module_name)
        gen_factory = getattr(module, 'ArithmeticProgression', None)
        if gen_factory is None: # 没有ArithmeticProgression类实现，则尝试aritgprog_gen函数实现
            gen_factory = getattr(module, 'aritprog_gen', None) 
            if gen_factory is None: # 没有aritprog函数实现, 则放弃。
                continue
        test(gen_factory, verbose)


'''
===========================================
Tests for arithmetic progression generators
===========================================
'''
def test(aritprog_gen, verbose=False):
    # Tests with built-in numeric types::
    ap = aritprog_gen(1, .5, 3)
    print( list(ap)) # [1.0, 1.5, 2.0, 2.5]
    ap = aritprog_gen(0, 1/3, 1)
    print( list(ap)) # [0.0, 0.3333333333333333, 0.6666666666666666]

    # Tests with standard library numeric types::
    from fractions import Fraction
    ap = aritprog_gen(0, Fraction(1, 3), 1)
    print( list(ap)) # [Fraction(0, 1), Fraction(1, 3), Fraction(2, 3)]
    from decimal import Decimal
    ap = aritprog_gen(0, Decimal('.1'), .3)
    print( list(ap)) # [Decimal('0'), Decimal('0.1'), Decimal('0.2')]

    # Test producing an empty series::
    ap = aritprog_gen(0, 1, 0)
    print( list(ap)) # []


if __name__ == '__main__':
    import sys
    main(sys.argv)
