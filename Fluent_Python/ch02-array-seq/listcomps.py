print('----------------')
symbols = 'A中国人'
codes = []
for symbol in symbols:
    codes.append(ord(symbol))
print(codes)

print('----------------')
symbols = 'A中国人'
codes = [ord(symbol) for symbol in symbols]
print(codes)

print('----------------')
x = 'my precious'
dummy = [x for x in 'ABC']
print(x)
print(dummy)

print('----------------')
x = 'ABC'
dummy = [ord(x) for x in x]
print(x)
print(dummy)

print('----------------')
symbols = 'A中国人'
beyond_ascii = [ord(s) for s in symbols if ord(s) > 127 ]
print(beyond_ascii)
beyond_ascii = list(filter(lambda c: c>127, map(ord, symbols)))
print(beyond_ascii)
