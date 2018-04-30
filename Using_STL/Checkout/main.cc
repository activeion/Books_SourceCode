#include <iostream>
#include <memory>
#include "customer.h"
#include "checkout.h"

int main(void)
{
    Checkout c;

    for(int i=0; i<5; ++i)
        c.addCustomer(std::make_unique<Customer>(2));

    for(int i=0; i< 100; ++i) {
        if(!i%3) c.addCustomer(std::make_unique<Customer>(2));
        c.timeTicks();
    }

    return 0;
}

