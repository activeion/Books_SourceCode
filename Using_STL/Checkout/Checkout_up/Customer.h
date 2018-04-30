// Defines a customer by their time to checkout
#ifndef CUSTOMER_H
#define CUSTOMER_H

class Customer
{
    private:
        size_t checkout_t_ {}; // Time to checkout

    public:
        explicit Customer(size_t st = 10) :checkout_t_{st}{}

        // Decrement time remaining to checkout
        Customer& time_decrement()
        {
            --checkout_t_;
            return *this;
        }
        bool done() { return checkout_t_ == 0; }
};
#endif
