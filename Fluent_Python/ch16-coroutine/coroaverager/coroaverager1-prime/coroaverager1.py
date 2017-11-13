# BEGIN DECORATED_AVERAGER
"""
A coroutine to compute a running average
"""

from coroutil import coroutine  # <4>

@coroutine  # <5> 增加了预激装饰器
def averager():  # <6>
    total = 0.0
    count = 0
    average = None
    while True:
        term = yield average
        total += term
        count += 1
        average = total/count
# END DECORATED_AVERAGER

coro_avg = averager()  # <1>
from inspect import getgeneratorstate
print(getgeneratorstate(coro_avg))  # <2> 'GEN_SUSPENDED'
print(coro_avg.send(10))  # <3> 10.0
print(coro_avg.send(30)) # 20.0
print(coro_avg.send(5)) # 15.0

