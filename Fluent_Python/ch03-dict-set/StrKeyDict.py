class StrKeyDict0(dict):
    def __missing__(self, key):
        if isinstance(key, str):#必须有，否则死循环在__missing__
            raise KeyError(key)
        return self[str(key)]
    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError: #拦截异常
            return default
    def __contains__(self, key): # `key in dict`调用本函数, 注意拼写，不是__contain__
        #return key in self.keys() or str(key) in self.keys()
        bool1=key in self.keys() 
        bool2=str(key) in self.keys()
        return bool1 or bool2

import collections
class StrKeyDict(collections.UserDict):
    def __missing__(self,key):
        if isinstance(key, str):
            raise KeyError(key)
        return self[str(key)]
    def __contains__(self, key): # `key in dict`
        return str(key) in self.data
    def __setitem__(self, key, item): # `update()` `=`
        self.data[str(key)] = item


def print_d(d):
    # tests for item retrieval using `d[key]` notation:
    print(d['2'])
    print(d[4])
    #print(d[1]) # L4 KeyError

    # tests for item retrieval using `d.get(key)` notation:
    print(d.get('2'))
    print(d.get(4))
    print(d.get(1, 'N/A'))

    # tests for the `in` operator:
    print('2' in d)
    print(2 in d)
    print(1 in d)

    # tests for update using a `dict` or a sequence of pairs:
    d.update({6:'six', '8':'eight'})
    print( sorted(d.keys(), key=int) )
    d.update( [(10, 'ten'), ('12', 'twelve')])
    print( sorted(d.keys(), key=int) )

d0 = StrKeyDict0([('2', 'two'), ('4', 'four')])
print_d(d0)

d = StrKeyDict([('2', 'two'), ('4', 'four')])
print_d(d)


print('---------------')
# 不可变的代理字典
from types import MappingProxyType
d = {1:'A'}
d_proxy = MappingProxyType(d)
print('before change:')
print(d_proxy)
print(d_proxy[1])

#d_proxy[2] = 'x'
d[2] = 'B'

print('after chnage:')
print(d_proxy)
print(d_proxy[2])



