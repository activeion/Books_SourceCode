"""
A 2-dimensional vector class

"""
from array import array
import math

# BEGIN VECTOR2D_V0 基本框架
class Vector2d:
    typecode = 'd'  # <1> 类变量

    def __init__(self, x, y):
        self.__x = float(x)    # <2> 今早发现x,y的非法性, 比如非数字
        self.__y = float(y)

    @property
    def x(self):
        return self.__x

    @property
    def y(self):
        return self.__y

    def __hash__(self):
        return hash(self.x) ^ hash(self.y)

    def __iter__(self):
        return (i for i in (self.__x, self.__y))  # <3> 用于拆包, for ... in 返回一个迭代器

    def __repr__(self):
        class_name = type(self).__name__
        return '{}({!r}, {!r})'.format(class_name, *self)  # <4>

    def __str__(self):
        return str(tuple(self))  # <5>

    def __bytes__(self):
        a = bytes([ord(self.typecode)])  # <6>
        b = bytes(array(self.typecode, self))  # <7>
        return a + b
        '''
        return (bytes([ord(self.typecode)]) +  # <6>
                bytes(array(self.typecode, self)))  # <7>
        '''

    def __eq__(self, other):
        return tuple(self) == tuple(other)  # <8>

    def __abs__(self):
        return math.hypot(self.__x, self.__y)  # <9>

    def __bool__(self):
        return bool(abs(self))  # <10>
# END VECTOR2D_V0

# BEGIN VECTOR2D_V1 备选的构造方法
    @classmethod # <1> 类函数，定义操作类的方法
    def frombytes(cls, octets): # <2>
        typecode = chr(octets[0]) # <3>
        memv = memoryview(octets[1:]).cast(typecode) # <4>
        return cls(*memv) # <5>
# END VECTOR2D_V1

# BEGIN VECTOR2D_V2 格式化显示
    def angle(self):
        return math.atan2(self.__y, self.__x)

    def __format__(self, fmt_spec=''):
        if fmt_spec.endswith('p'):
            fmt_spec = fmt_spec[:-1]
            coords = (abs(self), self.angle())
            outer_fmt = '<{}, {}>'
        else:
            coords = self
            outer_fmt = '({}, {})'
        components = (format(c, fmt_spec) for c in coords)
        return outer_fmt.format(*components)
# END VECTOR_V2

if __name__ == '__main__':

    # BEGIN VECTOR2D_V0_DEMO
    v1 = Vector2d(3, 4)
    print(v1.x, v1.y)  # <1> 3.0 4.0
    x, y = v1  # <2>
    print(x, y) # (3.0, 4.0)
    print(v1) # <3> Vector2d(3.0, 4.0)
    v1_clone = eval(repr(v1))  # <4> 照着v1原样克隆了一个v1_clone
    print(v1 == v1_clone)  # <5> True
    print(v1)  # <6> (3.0, 4.0)
    octets = bytes(v1)  # <7>
    print(octets) # b'd\\x00\\x00\\x00\\x00\\x00\\x00\\x08@\\x00\\x00\\x00\\x00\\x00\\x00\\x10@'
    print(abs(v1)) # <8> 5.0
    print(bool(v1), bool(Vector2d(0, 0))) # <9> (True, False)
    # END VECTOR2D_V0_DEMO

    # BEGIN VECTOR2D_V1_DEMO
    # 从字节序列串行化出来一个v1_clone，注意不是克隆出来的。
    v1_clone = Vector2d.frombytes(bytes(v1)) # Vector2d(3.0, 4.0)
    print(v1_clone)
    print(v1==v1_clone) # True
    # END VECTOR2D_V1_DEMO

    # BEGIN VECTOR2D_V2_DEMO
    #Tests of ``format()`` with Cartesian coordinates:
    print(format(v1)) # '(3.0, 4.0)'
    print(format(v1, '.2f')) # '(3.00, 4.00)'
    print(format(v1, '.3e')) # '(3.000e+00, 4.000e+00)' 

    #Tests of the ``angle`` method::
    print(Vector2d(0, 0).angle()) # 0.0
    print(Vector2d(1, 0).angle()) # 0.0
    epsilon = 10**-8
    print(abs(Vector2d(0, 1).angle() - math.pi/2) < epsilon) # True
    print(abs(Vector2d(1, 1).angle() - math.pi/4) < epsilon) # True

    #Tests of ``format()`` with polar coordinates:
    print(format(Vector2d(1, 1), 'p')) # doctest:+ELLIPSIS '<1.414213..., 0.785398...>'
    print(format(Vector2d(1, 1), '.3ep'))# '<1.414e+00, 7.854e-01>'
    print(format(Vector2d(1, 1), '0.5fp'))# '<1.41421, 0.78540>'
    # END VECTOR2D_V2_DEMO

    # BEGIN VECTOR2D_V3_DEMO
    #Tests of `x` and `y` read-only properties:
    print(v1.x, v1.y) # (3.0, 4.0)
    #v1.x = 123 # Traceback (most recent call last): ...  AttributeError: can't set attribute

    #Tests of hashing:
    v1 = Vector2d(3, 4)
    v2 = Vector2d(3.1, 4.2)
    print(hash(v1), hash(v2)) # (7, 384307168202284039)
    print(len(set([v1, v2]))) # 2
   # END VECTOR2D_V3_DEMO
