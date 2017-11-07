"""
A multi-dimensional ``Vector`` class, take 9: operator ``@``

WARNING: This example requires Python 3.5 or later.

"""

from array import array
import reprlib
import math
import functools
import operator
import itertools
import numbers


class Vector:
    typecode = 'd'

    def __init__(self, components):
        self._components = array(self.typecode, components)

    def __iter__(self):
        return iter(self._components)

    def __repr__(self):
        components = reprlib.repr(self._components)
        components = components[components.find('['):-1]
        return 'Vector({})'.format(components)

    def __str__(self):
        return str(tuple(self))

    def __bytes__(self):
        return (bytes([ord(self.typecode)]) +
                bytes(self._components))

    def __eq__(self, other):
        if isinstance(other, Vector):
            return (len(self) == len(other) and
                    all(a == b for a, b in zip(self, other)))
        else:
            return NotImplemented

    def __hash__(self):
        hashes = (hash(x) for x in self)
        return functools.reduce(operator.xor, hashes, 0)

    def __abs__(self):
        return math.sqrt(sum(x * x for x in self))

    def __bool__(self):
        return bool(abs(self))

    def __len__(self):
        return len(self._components)

    def __getitem__(self, index):
        cls = type(self)
        if isinstance(index, slice):
            return cls(self._components[index])
        elif isinstance(index, int):
            return self._components[index]
        else:
            msg = '{.__name__} indices must be integers'
            raise TypeError(msg.format(cls))

    shortcut_names = 'xyzt'

    def __getattr__(self, name):
        cls = type(self)
        if len(name) == 1:
            pos = cls.shortcut_names.find(name)
            if 0 <= pos < len(self._components):
                return self._components[pos]
        msg = '{.__name__!r} object has no attribute {!r}'
        raise AttributeError(msg.format(cls, name))

    def angle(self, n):
        r = math.sqrt(sum(x * x for x in self[n:]))
        a = math.atan2(r, self[n-1])
        if (n == len(self) - 1) and (self[-1] < 0):
            return math.pi * 2 - a
        else:
            return a

    def angles(self):
        return (self.angle(n) for n in range(1, len(self)))

    def __format__(self, fmt_spec=''):
        if fmt_spec.endswith('h'):  # hyperspherical coordinates
            fmt_spec = fmt_spec[:-1]
            coords = itertools.chain([abs(self)],
                                     self.angles())
            outer_fmt = '<{}>'
        else:
            coords = self
            outer_fmt = '({})'
        components = (format(c, fmt_spec) for c in coords)
        return outer_fmt.format(', '.join(components))

    @classmethod
    def frombytes(cls, octets):
        typecode = chr(octets[0])
        memv = memoryview(octets[1:]).cast(typecode)
        return cls(memv)

    def __add__(self, other):
        try:
            pairs = itertools.zip_longest(self, other, fillvalue=0.0)
            return Vector(a + b for a, b in pairs)
        except TypeError:
            return NotImplemented

    def __radd__(self, other):
        return self + other

    def __mul__(self, scalar):
        if isinstance(scalar, numbers.Real):
            return Vector(n * scalar for n in self)
        else:
            return NotImplemented

    def __rmul__(self, scalar):
        return self * scalar

    def __matmul__(self, other):
        try:
            return sum(a * b for a, b in zip(self, other))
        except TypeError:
            return NotImplemented

    def __rmatmul__(self, other):
        return self @ other  # this only works in Python 3.5

if __name__ == '__main__':
    # Basic tests of operator ``+``::
    v1 = Vector([3, 4, 5])
    v2 = Vector([6, 7, 8])
    print( v1 + v2) # Vector([9.0, 11.0, 13.0])
    print(v1 + v2 == Vector([3+6, 4+7, 5+8])) # True
    v3 = Vector([1, 2])
    print(v1 + v3) # short vectors are filled with 0.0 on addition # Vector([4.0, 6.0, 5.0])

    # Tests of ``+`` with mixed types::
    print(v1 + (10, 20, 30)) # Vector([13.0, 24.0, 35.0])
    from vector2d_v3 import Vector2d
    v2d = Vector2d(1, 2)
    print(v1 + v2d) # Vector([4.0, 6.0, 5.0])

    # Tests of ``+`` with mixed types, swapped operands::
    print((10, 20, 30) + v1) # Vector([13.0, 24.0, 35.0])
    from vector2d_v3 import Vector2d
    v2d = Vector2d(1, 2)
    print(v2d + v1) # Vector([4.0, 6.0, 5.0])

    # Tests of ``+`` with an unsuitable operand:
    print(v1 + 1) # Traceback (most recent call last): ...  TypeError: unsupported operand type(s) for +: 'Vector' and 'int'
    print(v1 + 'ABC') # Traceback (most recent call last): ...  TypeError: unsupported operand type(s) for +: 'Vector' and 'str'

    # Basic tests of operator ``*``::
    v1 = Vector([1, 2, 3])
    print(v1 * 10) # Vector([10.0, 20.0, 30.0])
    print(10 * v1) # Vector([10.0, 20.0, 30.0])

    # Tests of ``*`` with unusual but valid operands::
    print(v1 * True) # Vector([1.0, 2.0, 3.0])
    from fractions import Fraction
    print(v1 * Fraction(1, 3)) # doctest:+ELLIPSIS) # Vector([0.3333..., 0.6666..., 1.0])

    # Tests of ``*`` with unsuitable operands::
    print(v1 * (1, 2)) # Traceback (most recent call last): ...  TypeError: can't multiply sequence by non-int of type 'Vector'

    # Tests of operator `==`::
    va = Vector(range(1, 4))
    vb = Vector([1.0, 2.0, 3.0])
    print(va == vb) # True
    vc = Vector([1, 2])
    from vector2d_v3 import Vector2d
    v2d = Vector2d(1, 2)
    print(vc == v2d) # True
    print(va == (1, 2, 3)) # False

    # Tests of operator `!=`::
    print(va != vb) # False
    print(vc != v2d) # False
    print(va != (1, 2, 3)) # True

    # Tests for operator `@` (Python >= 3.5), computing the dot product::
    va = Vector([1, 2, 3])
    vz = Vector([5, 6, 7])
    print(va @ vz == 38.0) # 1*5 + 2*6 + 3*7) # True
    print([10, 20, 30] @ vz) # 380.0
    print(va @ 3) # Traceback (most recent call last): ...  TypeError: unsupported operand type(s) for @: 'Vector' and 'int'
