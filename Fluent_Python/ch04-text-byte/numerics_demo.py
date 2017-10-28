# BEGIN NUMERICS_DEMO
import unicodedata
import re

re_digit = re.compile(r'\d')

sample = '1\xbc\xb2\u0969\u136b\u216b\u2466\u2480\u3285'
sample2= '1\ubcb2\u0969\u136b\u216b\u2466\u2480\u3285'

def print_str(sample):
    for char in sample:
        print('U+%04x' % ord(char),                       # <1>
              char.center(6),                             # <2>
              're_dig' if re_digit.match(char) else '-',  # <3> 是正则中的数字吗？
              'isdig' if char.isdigit() else '-',         # <4> 是数字吗？
              'isnum' if char.isnumeric() else '-',       # <5> 是数值吗？
              format(unicodedata.numeric(char), '5.2f'),  # <6> 更多字符类型信息
              unicodedata.name(char),                     # <7> 更多字符类型信息
              sep='\t')

print_str(sample)
print_str(sample2)

# END NUMERICS_DEMO
