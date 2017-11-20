# strategy_best.py
# Strategy pattern -- function-based implementation
# selecting best promotion from static list of functions

from collections import namedtuple

Customer = namedtuple('Customer', 'name fidelity')


class LineItem:

    def __init__(self, product, quantity, price):
        self.product = product
        self.quantity = quantity
        self.price = price

    def total(self):
        return self.price * self.quantity


class Order:  # the Context

    def __init__(self, customer, cart, promotion=None):
        self.customer = customer
        self.cart = list(cart)
        self.promotion = promotion

    def total(self):
        if not hasattr(self, '__total'):
            self.__total = sum(item.total() for item in self.cart)
        return self.__total

    def due(self):
        if self.promotion is None:
            discount = 0
        else:
            discount = self.promotion(self)
        return self.total() - discount

    def __repr__(self):
        fmt = '<Order total: {:.2f} due: {:.2f}>'
        return fmt.format(self.total(), self.due())


def fidelity_promo(order):
    """5% discount for customers with 1000 or more fidelity points"""
    return order.total() * .05 if order.customer.fidelity >= 1000 else 0


def bulk_item_promo(order):
    """10% discount for each LineItem with 20 or more units"""
    discount = 0
    for item in order.cart:
        if item.quantity >= 20:
            discount += item.total() * .1
    return discount


def large_order_promo(order):
    """7% discount for orders with 10 or more distinct items"""
    distinct_items = {item.product for item in order.cart}
    if len(distinct_items) >= 10:
        return order.total() * .07
    return 0

# BEGIN STRATEGY_BEST

# best 1
promos = [fidelity_promo, bulk_item_promo, large_order_promo]  # <1>

# best 2
promos = [globals()[name] for name in globals()  # <1>
            if name.endswith('_promo')  # <2>
            and name != 'best_promo']   # <3>

# best 3
import inspect
import promotions
promos = [func for name, func in
                inspect.getmembers(promotions, inspect.isfunction)]

# END STRATEGY_BEST

def best_promo(order):  # <2>
    """Select best discount available
    """
    return max(promo(order) for promo in promos)  # <3>



############################################

joe = Customer('John Doe', 0)
ann = Customer('Ann Smith', 1100)
cart = [LineItem('banana', 4, .5),
     LineItem('apple', 10, 1.5),
     LineItem('watermellon', 5, 5.0)]
print(Order(joe, cart, fidelity_promo)) # <Order total: 42.00 due: 42.00>
print(Order(ann, cart, fidelity_promo)) # <Order total: 42.00 due: 39.90>

banana_cart = [LineItem('banana', 30, .5),
               LineItem('apple', 10, 1.5)]
print(Order(joe, banana_cart, bulk_item_promo)) # <Order total: 30.00 due: 28.50>

long_order = [LineItem(str(item_code), 1, 1.0)
              for item_code in range(10)]
print(Order(joe, long_order, large_order_promo)) # <Order total: 10.00 due: 9.30>
print(Order(joe, cart, large_order_promo)) # <Order total: 42.00 due: 42.00>

# BEGIN STRATEGY_BEST_TESTS

print(Order(joe, long_order, best_promo)) # <1> <Order total: 10.00 due: 9.30>
print(Order(joe, banana_cart, best_promo)) # <2> <Order total: 30.00 due: 28.50>
print(Order(ann, cart, best_promo)) # <3> <Order total: 42.00 due: 39.90>

# END STRATEGY_BEST_TESTS
