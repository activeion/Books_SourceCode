import random
import time

def stupid_fib(n):
    index = 0
    a = 0
    b = 1
    while index < n:
        sleep_cnt = yield b
        print('let me think {0} secs'.format(sleep_cnt))
        time.sleep(sleep_cnt)
        a, b = b, a + b
        index += 1

def copy_stupid_fib(n):
    print('I am copy from stupid fib')
    yield from stupid_fib(n)
    print('Copy end')

print('-'*10 + 'test yield from and send' + '-'*10)
csfib = copy_stupid_fib(20)
fib_res = next(csfib)
while True:
    print(fib_res)
    try:
        fib_res = csfib.send(random.uniform(0, 0.5))
    except StopIteration:
        break
