print('----------------')
symbols = 'A中国人'
codes = []
for symbol in symbols:
    codes.append(ord(symbol)) # ord() 返回相应编码的单个字符的数值
print(codes)

print('----------------')
symbols = 'A中国人'
codes = [ord(symbol) for symbol in symbols] # 更加简洁/易懂
print(codes)

# 列表推导有自己的作用域，内部变量x不影响外部的x
print('----------------')
x = 'my precious'
dummy = [x for x in 'ABC']
print(x)
print(dummy)

# 列表推导内部也可以重名，比如`... x in x`
print('----------------')
x = 'ABC'
dummy = [ord(x) for x in x]
print(x)
print(dummy)

# []会忽略其内部的换行符
# filter(func, seq) 返回func为true的元素组成的一个新list
# map(func, seq) 对seq的每一个元素都执行func操作
print('----------------')
symbols = 'A中国人'

beyond_ascii = [ord(s) for s in symbols if ord(s) > 127] # 比fileter/map简洁易懂
print(beyond_ascii)

beyond_ascii = list(filter(lambda c: c>127, map(ord, symbols)))
print(beyond_ascii)