// Supermarket checkout - maintains and processes customers in a queue
#ifndef CHECKOUT_H
#define CHECKOUT_H
#include <queue>                                 // For queue container
#include "Customer.h"

class Checkout
{
    private:
        std::queue<Customer> customers;                // The queue waiting to checkout
    public:
        //void add(const Customer& customer) { customers.push(customer); }
        void add(const Customer&& customer) { customers.push(customer); }
        size_t qlength() const { return customers.size(); }

        // Increment the time by one minute
        void time_increment()
        {
            if (!customers.empty())// or if(customers.size)) //不能 if(customers.front()!=0) //不存在null对象
            { // There are customers waiting...
                if (customers.front().time_decrement().done())  // If the customer is done...
                    customers.pop();                              // ...remove from the queue
            } else {
                std::cout << "customers queue is empty!" << std::endl;
            }
        };

        bool operator<(const Checkout& other) const { return qlength() < other.qlength(); }
        bool operator>(const Checkout& other) const { return qlength() > other.qlength(); }
};
#endif
