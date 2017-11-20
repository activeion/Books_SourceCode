"""
A multi-dimensional ``Vector`` class, take 1
"""

from array import array
import reprlib
import math
import numbers
import functools
import operator # 代替lambda的简短函数, add, sum, etc
import itertools # chain() VECTOR_V5

# BEGIN VECTOR_V1
class Vector:
    typecode = 'd'

    def __init__(self, components):
        self._components = array(self.typecode, components)  # <1>

    def __iter__(self):
        return iter(self._components)  # <2>

    def __repr__(self):
        components = reprlib.repr(self._components)  # <3>
        components = components[components.find('['):-1]  # <4>
        return 'Vector({})'.format(components)

    def __str__(self):
        return str(tuple(self))

    def __bytes__(self):
        return (bytes([ord(self.typecode)]) +
                bytes(self._components))  # <5>

    def __abs__(self):
        return math.sqrt(sum(x * x for x in self))  # <6>

    def __bool__(self):
        return bool(abs(self))

    @classmethod
    def frombytes(cls, octets):
        typecode = chr(octets[0])
        memv = memoryview(octets[1:]).cast(typecode)
        return cls(memv)  # <7>
# END VECTOR_V1

# BEGIN VECTOR_V2
    def __len__(self):
        return len(self._components)

    def __getitem__(self, index):
        # return self._components[index] # 最简单地支持切片，但返回array，不理想。
        cls = type(self)  # <1>
        if isinstance(index, slice):  # <2> 如果index是一个切片(slice对象)，即含有冒号
            return cls(self._components[index])  # <3>
        elif isinstance(index, numbers.Integral):  # <4> 如果index是一个数字，即没有含有冒号
            return self._components[index]  # <5>
        else:
            msg = '{cls.__name__} indices must be integers'
            raise TypeError(msg.format(cls=cls))  # <6>
# END VECTOR_V2

# BEGIN VECTOR_V3_GETATTR
    shortcut_names = 'xyzt'
    def __getattr__(self, name):
        cls = type(self)  # <1>
        if len(name) == 1:  # <2>
            pos = cls.shortcut_names.find(name)  # <3>
            if 0 <= pos < len(self._components):  # <4>
                return self._components[pos]
        msg = '{.__name__!r} object has no attribute {!r}'  # <5>
        raise AttributeError(msg.format(cls, name))
# END VECTOR_V3_GETATTR
# BEGIN VECTOR_V3_SETATTR
    def __setattr__(self, name, value):
        cls = type(self)
        if len(name) == 1:  # <1>
            if name in cls.shortcut_names:  # <2>
                error = 'readonly attribute {attr_name!r}'
            elif name.islower():  # <3>
                error = "can't set attributes 'a' to 'z' in {cls_name!r}"
            else:
                error = ''  # <4>
            if error:  # <5>
                msg = error.format(cls_name=cls.__name__, attr_name=name)
                raise AttributeError(msg)
        super().__setattr__(name, value)  # <6>
# END VECTOR_V3_SETATTR

# BEGIN VECTOR_V4
    def __eq__(self, other):
        # return tuple(self) == tuple(other) # multi vector, 效率低下
        # return len(self)==len(other) and all(a==b for a,b in zip(self, other)) # 效果同下
        if len(self) != len(other):
            return False
        for a,b in zip(self, other): # zip()函数生成一个由双元素的元组构成的一个惰性生成器
            if a!=b:
                return False
        return True

    def __hash__(self):
        # hashes = map(hash, self._components) # 效果同下
        hashes = ( hash(x) for x in self._components ) # 创建genexp，惰性计算
        return functools.reduce(operator.xor, hashes, 0) # 0为+, |, ^操作的初始值；1为*，&操作的初始值
# END VECTOR_V4

# BEGIN VECTOR_V5
    def angle(self, n):  # <2>
        r = math.sqrt(sum(x * x for x in self[n:]))
        a = math.atan2(r, self[n-1])
        if (n == len(self) - 1) and (self[-1] < 0):
            return math.pi * 2 - a
        else:
            return a

    def angles(self):  # <3>
        return (self.angle(n) for n in range(1, len(self)))

    def __format__(self, fmt_spec=''):
        if fmt_spec.endswith('h'):  # hyperspherical coordinates
            fmt_spec = fmt_spec[:-1]
            coords = itertools.chain([abs(self)],
                                     self.angles())  # <4>
            outer_fmt = '<{}>'  # <5>
        else:
            coords = self
            outer_fmt = '({})'  # <6>
        components = (format(c, fmt_spec) for c in coords)  # <7>
        return outer_fmt.format(', '.join(components))  # <8>
# END VECTOR_V5

if __name__ == '__main__':
    print('-------------------------')
    # Tests with 3-dimensions::
    v1 = Vector([3, 4, 5])
    x, y, z = v1
    print( x, y, z ) # (3.0, 4.0, 5.0)
    print( v1 ) # Vector([3.0, 4.0, 5.0])
    v1_clone = eval(repr(v1))
    print( v1 == v1_clone ) # True
    print(v1)#(3.0, 4.0, 5.0)
    print( abs(v1) )# doctest:+ELLIPSIS 7.071067811...
    print( bool(v1), bool(Vector([0, 0, 0])) ) # (True, False)

    # Tests with many dimensions::
    v7 = Vector(range(7))
    print( v7 ) # Vector([0.0, 1.0, 2.0, 3.0, 4.0, ...])
    print( abs(v7)) # doctest:+ELLIPSIS 9.53939201...

    # Test of ``.__bytes__`` and ``.frombytes()`` methods::
    v1 = Vector([3, 4, 5])
    v1_clone = Vector.frombytes(bytes(v1))
    print( v1_clone ) # Vector([3.0, 4.0, 5.0])
    print( v1 == v1_clone ) # True

    # BEGIN VECTOR_V2_DEMO
    # Tests of sequence behavior:: 需要添加__len__ __getitem__方法
    v1 = Vector([3, 4, 5])
    print( len(v1) ) # 3
    print( v1[0], v1[len(v1)-1], v1[-1] ) # (3.0, 5.0, 5.0)

    # Test of slicing::
    v7 = Vector(range(7))
    print( v7[-1] )# <1> 6.0
    print( v7[1:4]) # <2> Vector([1.0, 2.0, 3.0])
    print( v7[-1:]) # <3> Vector([6.0])
    #v7[1,2]  # <4> Traceback (most recent call last): ...  TypeError: Vector indices must be integers

    # END VECTOR_V2_DEMO

    # BEGIN VECTOR_V3_DEMO
    #Tests of dynamic attribute access::
    v7 = Vector(range(10))
    print( v7.x ) #  0.0
    print( v7.y, v7.z, v7.t ) # (1.0, 2.0, 3.0)

    # Dynamic attribute lookup failures::
    # print( v7.k ) # Traceback (most recent call last): ...  AttributeError: 'Vector' object has no attribute 'k'
    v3 = Vector(range(3))
    # print( v3.t ) # Traceback (most recent call last): ...  AttributeError: 'Vector' object has no attribute 't'
    # print( v3.spam) # Traceback (most recent call last): ...  AttributeError: 'Vector' object has no attribute 'spam'

    # Tests of preventing attributes from 'a' to 'z'::
    # v1.x = 7 # Traceback (most recent call last): ...  AttributeError: readonly attribute 'x'
    # v1.w = 7 # Traceback (most recent call last): ...  AttributeError: can't set attributes 'a' to 'z' in 'Vector'

    # Other attributes can be set::
    v1.X = 'albatross'
    print( v1.X ) # 'albatross'
    v1.ni = 'Ni!'
    print(v1.ni) # 'Ni!'
    # END VECTOR_V3_DEMO

    # END VECTOR_V4_DEMO
    v1 = Vector([1,2,3,4,5]) # Vector(1,2,3,4,5)写法是错误的
    v2 = Vector([6,7,8,9,10])
    v3 = Vector({16,17,18,19,20}) # {}是set，iterable,与[]等效
    print(hash(v1), hash(v2))
    s1 = set([v1,v2]) # 由于Vector有__hash__方法，可散列，因此可以放入集合中
    print(s1) # {Vector([1.0, 2.0, 3.0, 4.0, 5.0]), Vector([6.0, 7.0, 8.0, 9.0, 10.0])}
    # END VECTOR_V4_DEMO

    # BEGIN VECTOR_V5_DEMO
    # Tests of ``format()`` with Cartesian coordinates in 2D::
    v1 = Vector([3, 4])
    print(format(v1) ) # '(3.0, 4.0)'
    print(format(v1, '.2f') ) # '(3.00, 4.00)'
    print( format(v1, '.3e') ) # '(3.000e+00, 4.000e+00)'

    # Tests of ``format()`` with Cartesian coordinates in 3D and 7D::
    v3 = Vector([3, 4, 5])
    print( format(v3) ) # '(3.0, 4.0, 5.0)'
    print( format(Vector(range(7))) ) # '(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0)'

    # Tests of ``format()`` with spherical coordinates in 2D, 3D and 4D::
    print(format(Vector([1, 1]), 'h')) # doctest:+ELLIPSIS '<1.414213..., 0.785398...>'
    print( format(Vector([1, 1]), '.3eh') ) # '<1.414e+00, 7.854e-01>'
    print( format(Vector([1, 1]), '0.5fh') ) # '<1.41421, 0.78540>'
    print( format(Vector([1, 1, 1]), 'h') )# doctest:+ELLIPSIS '<1.73205..., 0.95531..., 0.78539...>'
    print( format(Vector([2, 2, 2]), '.3eh') ) # '<3.464e+00, 9.553e-01, 7.854e-01>'
    print( format(Vector([0, 0, 0]), '0.5fh') ) # '<0.00000, 0.00000, 0.00000>'
    print( format(Vector([-1, -1, -1, -1]), 'h')) # doctest:+ELLIPSIS '<2.0, 2.09439..., 2.18627..., 3.92699...>'
    print( format(Vector([2, 2, 2, 2]), '.3eh') ) # '<4.000e+00, 1.047e+00, 9.553e-01, 7.854e-01>'
    print( format(Vector([0, 1, 0, 0]), '0.5fh') ) # '<1.00000, 1.57080, 0.00000, 0.00000>'
    # END VECTOR_V5_DEMO
