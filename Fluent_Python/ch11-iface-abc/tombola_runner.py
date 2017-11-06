# BEGIN TOMBOLA_RUNNER

from tombola_class.tombola import Tombola

# modules to test
from  tombola_class.bingo import * # <1>
from  tombola_class.lotto import * # <1>
from  tombola_class.drum import * # <1>
from  tombola_class.tombolist import * # <1>

def test(ConcreteTombola):
    # Create and load instance from iterable::
    balls = list(range(3))
    globe = ConcreteTombola(balls)
    print( globe.loaded() ) # True
    print( globe.inspect()) # (0, 1, 2)

    # Pick and collect balls::
    picks = []
    picks.append(globe.pick())
    picks.append(globe.pick())
    picks.append(globe.pick())

    # Check state and results::
    print( globe.loaded()) # False
    print( sorted(picks) == balls) # True

    # Reload::
    globe.load(balls)
    print( globe.loaded()) # True
    picks = [globe.pick() for i in balls]
    print( globe.loaded()) # False


    # Check that `LookupError` (or a subclass) is the exception thrown when the device is empty::
    globe = ConcreteTombola([])
    try:
        globe.pick()
    except LookupError as exc:
        print('OK')


    # Load and pick 100 balls to verify that they all come out::
    balls = list(range(100))
    globe = ConcreteTombola(balls)
    picks = []
    while globe.inspect():
        picks.append(globe.pick())
    print( len(picks) == len(balls)) # True
    print( set(picks) == set(balls)) # True


    # Check that the order has changed and is not simply reversed::

    print( picks != balls) # True
    print( picks[::-1] != balls) # True


    '''
    Note: the previous 2 tests have a *very* small chance of failing
    even if the implementation is OK. The probability of the 100
    balls coming out, by chance, in the order they were inspect is
    1/100!, or approximately 1.07e-158. It's much easier to win the
    Lotto or to become a billionaire working as a programmer.
    '''
# END TOMBOLA_RUNNER

if __name__ == '__main__':
    real_subclasses = Tombola.__subclasses__()
    virtual_classes = list(Tombola._abc_registry)

    for cls in real_subclasses + virtual_classes:
        test(cls)
