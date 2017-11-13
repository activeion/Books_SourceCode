def simple_coroutine():
    print('-> coroutine started')
    x = yield
    print('-> coroutine received:', x)

my_coro = simple_coroutine()
print(my_coro) # <generator object simple_coroutine at 0x7f13cdf7ddb0>

try:
    my_coro.send(42) # TypeError
except TypeError:
    print('TypeError')

next(my_coro) # -> coroutine started. 预激活(prime)

try:
    my_coro.send(42) # -> coroutine received: 42 Trackback...StopIteration
except StopIteration:
    print('StopIteration')

#########################################

def simple_coroutine2(a):
    print('-> coroutine start: a=', a)
    b = yield
    print('-> coroutine received: b=', b)
    c= yield a + b
    print('-> coroutine received: c = ', c)

my_coro2 = simple_coroutine2(14)
from inspect import getgeneratorstate
print(getgeneratorstate(my_coro2))
next(my_coro2)
print(getgeneratorstate(my_coro2))
my_coro2.send(28)
try:
    my_coro2.send(99)
except StopIteration:
    print('Stop___Iteration')
print(getgeneratorstate(my_coro2))


