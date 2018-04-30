#include <list>
#include <deque>
#include <memory>
class Customer;

//typedef std::unique_ptr<Customer> UpCustomer;
using UpCustomer = std::unique_ptr<Customer>;
using UpCustomers = std::deque<UpCustomer>;
class Checkout
{
    private:
        //std::list<UpCustomer> customers_;
        UpCustomers customers_;
    public:
    void addCustomer(UpCustomer&& customer)
    {
        customers_.push_back(std::move(customer));
    }

    void timeTicks(void)
    {
        if(customers_.size())
            customers_.front()->set_checking(true);
        for(UpCustomer& c : customers_) {
            c->timeTicks();
        }
        if(customers_.size()) {
            if(customers_.front()->Done()) {
                UpCustomer& up = customers_.front();
                std::cout << "customer checkout after waiting " 
                    << up->line_time()
                    << "min"<<std::endl;
                customers_.pop_front();
            }
        }
    }
};

