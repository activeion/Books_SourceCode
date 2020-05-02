symbols='A中国人'
t=tuple(ord(symbol) for symbol in symbols)
print(t)

print('---------')
import array
a = array.array('I', (ord(symbol) for symbol in symbols))
print(a)

