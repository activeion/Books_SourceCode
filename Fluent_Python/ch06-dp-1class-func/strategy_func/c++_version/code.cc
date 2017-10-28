#include <iostream>
#include <string>
#include <vector>
#include <functional>

class Customer
{
    public:
    Customer(std::string name, int fidelity)
        : name_(name), fidelity_(fidelity) 
    {}
    std::string name_;
    int fidelity_;
};

class LineItem
{
    public:
    LineItem(std::string name, int quantity, double price):name_(name), quantity_(quantity), price_(price)
    {}
    double total() { return price_*quantity_; }

    private:
    std::string name_;
    int quantity_;
    double price_;
};

typedef std::vector<LineItem> Cart;
class Order;
typedef double(*Fun)(Order*);
class Order
{
    public:
        Order(Customer customer, Cart cart, Fun func)
            : customer_(customer), cart_(cart), func_(func){}
        double due()
        {
            return func_(this);
        }

        double due_function()
        {
            return function_();
    private:
        Customer customer_;
        Cart cart_;
        Fun func_;

        std::function function_;
};


double fidelity_promo(Order* order)
{
    return 1.6;
}

double bulk_promo(Order* order)
{
    return 1.9;
}

double large_order_promo(Order* order)
{
    return 2.6;
}

double fidelity_function(Order* order, int extra)
{
    return 11.4;
}

int main(void)
{
    Customer customer("jizh", 100);
    Cart cart;
    LineItem apples("apple", 5, 2.3);
    cart.push_back(apples);

    //利用函数指针实现策略模式
    Order order(customer, cart, fidelity_promo);
    order.due();
    Order order(customer, cart, bulk_promo);
    order.due();
    Order order(customer, cart, large_order);
    order.due();

    //利用function实现策略模式


    return 0;
}
