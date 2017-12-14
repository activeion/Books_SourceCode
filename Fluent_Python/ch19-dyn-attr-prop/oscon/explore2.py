"""

explore2.py: Script to explore the OSCON schedule feed

"""

from collections import abc
from keyword import iskeyword


class FrozenJSON:
    """A read-only façade for navigating a JSON-like object
       using attribute notation
    """

    # BEGIN EXPLORE2
    """
    @classmethod
    def build(cls, obj):  # <5>
        if isinstance(obj, abc.Mapping):  # <6>
            return cls(obj)
        elif isinstance(obj, abc.MutableSequence):  # <7>
            return [cls.build(item) for item in obj]
        else:  # <8>
            return obj
    """
    def __new__(cls, arg):  # <1>
        if isinstance(arg, abc.Mapping):
            return super().__new__(cls)  # <2>
        elif isinstance(arg, abc.MutableSequence):  # <3>
            return [cls(item) for item in arg]
        else:
            return arg
    # END EXPLORE2

    # BEGIN EXPLORE1
    def __init__(self, mapping):
        # self.__data = dict(mapping)  # <1>
        self.__data = {}
        for key, value in mapping.items():
            if iskeyword(key):
                key += '_'
            self.__data[key] = value
    # END EXPLORE1

    def __getattr__(self, name): # 类中没有找到name这个成员变量, 则调用本函数, name是一个str, missing attr accessed
        if hasattr(self.__data, name):
            return getattr(self.__data, name)
        else:
            # return FrozenJSON.build(self.__data[name])  # <4>
            return FrozenJSON(self.__data[name])  # <4>

if __name__ == '__main__':
    from osconfeed import load
    raw_feed = load()
    feed = FrozenJSON(raw_feed)
    print(len(feed.Schedule.speakers)) # 357
    print(sorted(feed.Schedule.keys())) # ['conferences', 'events', 'speakers', 'venues']
    print(feed.Schedule.speakers[-1].name) # 'Carina C. Zona'
    talk = feed.Schedule.events[40]
    print(talk.name) # 'There *Will* Be Bugs'
    print(talk.speakers) # [3471, 5199]
    # talk.flavor # Traceback (most recent call last): ...  KeyError: 'flavor'

