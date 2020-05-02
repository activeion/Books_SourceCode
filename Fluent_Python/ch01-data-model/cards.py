import collections
Card = collections.namedtuple('Card_jizh', ['rank', 'suit']) #注意名字可以任意写，print(Card)语句会打印'Card_card_car'

class FrenchDeck:
    ranks = [str(n) for n in range(2,11)] + list('JQKA')
    suits = 'spades diamonds clubs hearts'.split()

    def __init__(self):
        self._cards = [Card(rank, suit) 
            for suit in self.suits 
                for rank in self.ranks]

    def __len__(self):
        return len(self._cards)
    
    def __getitem__(self, position):
        return self._cards[position]


beer_card=Card('7', 'diamonds')
deck= FrenchDeck()
l=len(deck)


''' __getitem__() '''
print(deck[0])
print(deck[-1])

from random import choice
print(choice(deck))
print(choice(deck))
print(choice(deck))

print(deck[:3])   # 最上面三张牌 0, 1, 2
print(deck[12::13]) # 12 26 40 54

for card in deck: # 可迭代
    print(card)

# deck没有__contains__, in操作就使用顺序遍历
print(Card('Q', 'hearts') in deck)
print(Card('7', 'beasts') in deck)

# 排序
suit_values = dict(spades=3, hearts=2, diamonds=1, clubs=0)
def spades_high(card):
    rank_value=FrenchDeck.ranks.index(card.rank) 
    return rank_value * len(suit_values) + suit_values[card.suit]
for card in sorted(deck, key=spades_high):
    print(card)
    

