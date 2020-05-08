#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat May  2 14:43:56 2020

@author: jizh
"""

lax_coordinates=(33.9425, -118.408056)
city, year, pop, chg, area = ('Tokyo', 2003, 32450, 0.66, 8014)# 元组拆包

traveler_ids = [('USA', '31195855'), ('BRA', 'CE342567'), ('ESP', 'XDA205856')]
for passport in sorted(traveler_ids):
    print('%s/%s' % passport, end='\t') # 元组拆包
    print(f'{passport}')

print('------')
for country, _ in traveler_ids: # _ 占位符
    print(country)

'''
The most visible form of tuple unpacking is parallel assignment; that is, assigning items
from an iterable to a tuple of variables, as you can see in this example:
'''
lax_coordinates = (33.9425, -118.408056)
latitude, longitude = lax_coordinates # tuple unpacking
print(latitude)#33.9425
print( longitude )#-118.408056
'''
An elegant application of tuple unpacking is swapping the values of variables without
using a temporary variable:
'''
a,b=1,2
b, a = a, b
'''
Another example of tuple unpacking is prefixing an argument with a star when calling
a function:
'''
print(divmod(20, 8))#(2, 4)
t = (20, 8)
print(divmod(*t))#(2, 4)
quotient, remainder = divmod(*t)
print( (quotient, remainder) )#(2, 4)
'''
The preceding code also shows a further use of tuple unpacking: enabling functions to
return multiple values in a way that is convenient to the caller. For example, the
os.path.split() function builds a tuple (path, last_part) from a filesystem path:
'''
import os
_, filename = os.path.split('/home/luciano/.ssh/idrsa.pub')
print(filename)#'idrsa.pub'
'''
Sometimes when we only care about certain parts of a tuple when unpacking, a dummy
variable like _ is used as placeholder, as in the preceding example.
'''


'''
Nested Tuple Unpacking
The tuple to receive an expression to unpack can have nested tuples, like (a, b, (c,
d)), and Python will do the right thing if the expression matches the nesting structure.
Example 2-8 shows nested tuple unpacking in action.

Example 2-8. Unpacking nested tuples to access the longitude
'''

metro_areas = [
    ('Tokyo', 'JP', 36.933, (35.689722, 139.691667)), #1
    ('Delhi NCR', 'IN', 21.935, (28.613889, 77.208889)),
    ('Mexico City', 'MX', 20.142, (19.433333, -99.133333)),
    ('New York-Newark', 'US', 20.104, (40.808611, -74.020386)),
    ('Sao Paulo', 'BR', 19.649, (-23.547778, -46.635833)),
]

print('{:15} | {:^9} | {:^9}'.format('', 'lat.', 'long.'))
fmt = '{:15} | {:9.4f} | {:9.4f}'
for name, cc, pop, (latitude, longitude) in metro_areas: #2
      if longitude <= 0: #3
        print(fmt.format(name, latitude, longitude))

#1 Each tuple holds a record with four fields, the last of which is a coordinate pair.
#2 By assigning the last field to a tuple, we unpack the coordinates.
#3 if longitude <= 0: limits the output to metropolitan areas in the Western hemisphere.

'''
The output of Example 2-8 is:
            | lat. | long.
Mexico City | 19.4333 | -99.1333
New York-Newark | 40.8086 | -74.0204
Sao Paulo | -23.5478 | -46.6358
''' 



'''
Recall how we built the Card class in Example 1-1 in Chapter 1:
      Card = collections.namedtuple('Card', ['rank', 'suit'])

Example 2-9 shows how we could define a named tuple to hold information about a
city.

Example 2-9. Defining and using a named tuple type
'''
from collections import namedtuple
City = namedtuple('CityCity', 'name country population coordinates')
tokyo = City('Tokyo', 'JP', 36.933, (35.689722, 139.691667)) # can't use 'CityCity' instead of 'City'
print(tokyo) # City(name='Tokyo', country='JP', population=36.933, coordinates=(35.689722, 139.691667))
print(tokyo.population) # 36.933
print( tokyo.coordinates) # (35.689722, 139.691667)
print( tokyo[1] ) # 'JP'
#1 Two parameters are required to create a named tuple: a class name and a list of field names, which can be given as an iterable of strings or as a single space- delimited string.
#2 Data must be passed as positional arguments to the constructor (in contrast, the tuple constructor takes a single iterable).
#3 You can access the fields by name or position.
'''
A named tuple type has a few attributes in addition to those inherited from tuple.
Example 2-10 shows the most useful: the _fields class attribute, the class method
_make(iterable), and the _asdict() instance method.

Example 2-10. Named tuple attributes and methods (continued from the previous ex‐
ample)
'''
print(City._fields) #1 ('name', 'country', 'population', 'coordinates')
LatLong = namedtuple('LatLong', 'lat long')
delhi_data = ('Delhi NCR', 'IN', 21.935, LatLong(28.613889, 77.208889))
delhi = City._make(delhi_data) #2
print(delhi._asdict()) #3 OrderedDict([('name', 'Delhi NCR'), ('country', 'IN'), ('population', 21.935), ('coordinates', LatLong(lat=28.613889, long=77.208889))])
for key, value in delhi._asdict().items():
    print(key + ':', value)

#1 _fields is a tuple with the field names of the class.
#2 _make() allow you to instantiate a named tuple from an iterable; City(*del hi_data) would do the same.
#3 _asdict() returns a collections.OrderedDict built from the named tuple instance. That can be used to produce a nice display of city data.

'''
Now that we’ve explored the power of tuples as records, we can consider their second role as an immutable variant of the list type.
'''
