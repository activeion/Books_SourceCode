
# BEGIN BINGO

import random

class BingoCage:

    def __init__(self, items):
        self._items = list(items)  # <1>
        random.shuffle(self._items)  # <2>

    def pick(self):  # <3>
        try:
            return self._items.pop()
        except IndexError:
            raise LookupError('pick from empty BingoCage')  # <4>

    def __call__(self):  # <5>
        return self.pick()

bingo = BingoCage(range(3))

print(bingo.pick()) # 0
print(bingo())      # 2
print(callable(bingo)) # True
print(callable(bingo())) # False
print(callable(BingoCage)) # True
print(callable(BingoCage(   (1, 2, 4)   ))) # True



# END BINGO
