#adapted from Alex Martelli's example in "Re-learning Python"
# http://www.alex.it/python/accu04_Relearn_Python_alex.pdf
# (slide 41) Ex: lines-by-word file index

# BEGIN INDEX0
"""build an index mapping word -> list of occurrences """

import sys
import re
import collections

WORD_RE = re.compile('\w+')

"""
index = {}
"""
index = collections.defaultdict(list) # defaultdict是dict的子类
with open(sys.argv[1], encoding='utf-8') as fp:
    for line_no, line in enumerate(fp, 1): # 起始行行数为1，而不是默认的0
        for match in WORD_RE.finditer(line):
            word = match.group()
            column_no = match.start() + 1
            location = (line_no, column_no)
            # this is ugly; coded like this to make a point
            """ with index = {}
            occurrences = index.get(word,[]) # []表示找不到则返回[]
            occurrences.append(location)
            index[word]  = occurrences
            """
            """ with index = {}
            index.setdefault(word,[]).append(location)
            """
            index[word].append(location)

# print in alphabetical order
for word in sorted(index, key=str.upper):
    print(word, index[word])

