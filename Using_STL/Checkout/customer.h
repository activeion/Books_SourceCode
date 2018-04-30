// Defines a customer by their time to checkout
#ifndef CUSTOMER_H
#define CUSTOMER_H

class Customer
{
    private:
        size_t checkout_time_; // Time to checkout
        bool checking_;
        size_t line_time_;

    public:
        explicit Customer(size_t ct = 10) :checkout_time_(ct), line_time_{0}, checking_(false){}

        bool set_checking(bool flag) { bool old = checking_; checking_ = flag; return old; }
        size_t line_time() {return line_time_;} 
        // Decrement time remaining to checkout
        Customer& timeTicks()
        {
            if(checking_)
                --checkout_time_;
            ++line_time_;
            return *this;
        }
        bool Done() { return checkout_time_ == 0; }
};
#endif
