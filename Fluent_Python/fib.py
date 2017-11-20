# 每一次调用仅仅生产一个元素，消耗一个元素的内存
def fib(n):
    index = 0
    a = 0
    b = 1
    while index < n:
        yield b
        a, b = b, a + b
        index += 1

# 打印所有元素
print('-'*10 + 'test yield fib' + '-'*10)
n = 0
for fib_res in fib(20):
    n = n + 1
    if n==19:
        print(fib_res)
        break

n = 0
fib_gen = fib(20)
while True:
    n = n+1
    ret = next(fib_gen)
    if n==19:
        break
print(ret)
