#include <iostream> // For standard streams
#include <istream>
#include <iomanip> // For stream manipulators
#include <array>   // For array<T,N>
#include <numeric> // For std::iota()

static void cin_state(void)
{
  std::cout << "[fail,eof, bad]: [" 
    << std::cin.fail() << std::cin.eof() << std::cin.bad() 
    << "], state-good: " << std::cin.good() << std::endl;
}
#include <limits>
static void cin_clear(void)
{
  std::cout << " => before clear(), peek="<<std::cin.peek() << " :";
  cin_state();

  std::cin.clear(); // -- lead to goodbit

  std::cout << " after clear(): ";
  cin_state();

}
namespace jizh01
{
  // Ex2_01.cpp
  /*
Using array<T,N> to create a Body Mass Index (BMI) table
BMI = weight/(height*height)
weight is in kilograms, height is in meters
*/

  void test(void)
  {
    const unsigned int min_wt{100U}; // Minimum weight in table in lbs
    const unsigned int max_wt{250U}; // Maximum weight in table in lbs
    const unsigned int wt_step{10U}; // Weight increment
    const size_t wt_count{1 + (max_wt - min_wt) / wt_step};

    const unsigned int min_ht{48U}; // Minimum height in table in inches
    const unsigned int max_ht{84U}; // Maximum height in table in inches
    const unsigned int ht_step{2U}; // Height increment
    const size_t ht_count{1 + (max_ht - min_ht) / ht_step};

    const double lbs_per_kg{2.20462}; // Conversion factor lbs to kg
    const double ins_per_m{39.3701};  // Conversion factor ins to m

    std::array<unsigned int, wt_count> weight_lbs;
    std::array<unsigned int, ht_count> height_ins;

    // 用连续的递增来初始化一个容器, 本例没有用处。
    std::iota(std::begin(weight_lbs), std::end(weight_lbs), 0);
    std::iota(std::begin(height_ins), std::end(height_ins), 0);

    // Create weights from 100lbs in steps of 10lbs
    for (size_t i{}, w{min_wt}; i < wt_count; w += wt_step, ++i)
    {
      weight_lbs.at(i) = w;
    }
    // Create heights from 48 inches in steps of 2 inches
    unsigned int h{min_ht};
    for (auto &height : height_ins)
    {
      height = h;
      h += ht_step;
    }

    // Output table headings
    std::cout << std::setw(7) << " |";
    for (const auto& w : weight_lbs)
      std::cout << std::setw(5) << w << "  |";
    std::cout << std::endl;

    // Output line below headings
    for (size_t i {1}; i < wt_count; ++i)
      std::cout << "---------";
    std::cout << std::endl;

    double bmi{};          // Stores BMI
    unsigned int feet{};   // Whole feet for output
    unsigned int inches{}; // Whole inches for output
    const unsigned int inches_per_foot{12U};
    for (const auto &h : height_ins)
    {
      feet = h / inches_per_foot;
      inches = h % inches_per_foot;
      std::cout << std::setw(2) << feet << "'" << std::setw(2) << inches << "\""
                << "|";
      std::cout << std::fixed << std::setprecision(1);
      for (const auto &w : weight_lbs)
      {
        bmi = h / ins_per_m;
        bmi = (w / lbs_per_kg) / (bmi * bmi);
        std::cout << std::setw(2) << " " << bmi << " |";
      }
      std::cout << std::endl;
    }
    // Output line below table
    for (size_t i{1}; i < wt_count; ++i)
      std::cout << "---------";
    std::cout << "\nBMI from 18.5 to 24.9 is normal" << std::endl;
  }

} // namespace jizh01

#include <iostream> // For standard streams
#include <string> // For string types
#include <algorithm> // For swap() and copy() functions
#include <vector> // For vector (and iterators)
#include <iterator> // For front_insert_iterator & stream iterators

namespace jizh02
{

  // Ex2_02A.cpp
  // Sorting strings in a vector container
  using std::string;
  using std::vector;

  // Our very own algorithm to sort a range specified by iterators
  template <typename RandomIter>
  void bubble_sort(RandomIter start, RandomIter last)
  {
    std::cout << "Starting sort." << std::endl;
    bool out_of_order{false}; // true when values are not in order
    while (true)
    {
      for (auto first = start + 1; first != last; ++first)
      {
        if (*(first - 1) > *first)
        { // Out of order so swap them
          std::swap(*first, *(first - 1));
          out_of_order = true;
        }
      }
      if (!out_of_order)    // If they are in order (no swaps necessary)...
        break;              // ...we are done...
      out_of_order = false; // ...otherwise, go round again.
    }
  }

  void test(void)
  {
    cin_clear();

    vector<string> words; // Stores words to be sorted
    words.reserve(10);    // Allocate some space for elements
    std::cout << "Enter words separated by spaces. Enter Ctrl+Z on a separate line to end:" << std::endl;
    std::copy(std::istream_iterator<string>{std::cin},
              std::istream_iterator<string>{},
              std::back_inserter(words));

    bubble_sort(std::begin(words), std::end(words)); // Sort the words array

    // Output the sorted vector
    std::cout << "your words in ascending sequence:" << std::endl;
    std::copy(std::begin(words), std::end(words), std::ostream_iterator<string>{std::cout, " "});
    std::cout << std::endl;

    // Create a new vector by moving elements from words vector
    vector<string> words_copy{std::make_move_iterator(std::begin(words)),
                              std::make_move_iterator(std::end(words))};
    std::cout << "\nAfter moving elements from words, words_copy contains:" << std::endl;
    std::copy(std::begin(words_copy), std::end(words_copy), std::ostream_iterator<string>{std::cout, " "});
    std::cout << std::endl;

    // See what's happened to elements in words vector...
    std::cout << "\nwords vector has " << words.size() << " elements\n";
    if (words.front().empty())
      std::cout << "First element of container words is empty string object." << std::endl;

    std::cout << "First element is \"" << words.front() << "\"" << std::endl;
  }

} // namespace jizh02

#include <iostream> // For standard streams
#include <vector> // For vector container
namespace jizh03
{

  // Ex2_03.cpp
  // Understanding how capacity is increased in a vector container

  void test(void)
  {
    std::vector<size_t> sizes;      // Record numbers of elements
    std::vector<size_t> capacities; // and corresponding capacities
    size_t el_incr{10};             // Increment to initial element count
    size_t incr_count{4 * el_incr}; // Number of increments to element count

    for (size_t n_elements{}; n_elements < incr_count; n_elements += el_incr)
    {
      std::vector<int> values(n_elements);
      std::cout << "\nAppending to a vector with " << n_elements << " initial elements:\n";
      sizes.push_back(values.size());
      size_t space{values.capacity()};
      capacities.push_back(space);

      // Append elements to obtain capacity increases
      size_t count{}; // Counts capacity increases
      size_t n_increases{10};
      while (count < n_increases)
      {
        values.push_back(22);          // Append a new element
        if (space < values.capacity()) // Capacity increased...
        {                              // ...so record size and capacity
          space = values.capacity();
          capacities.push_back(space);
          sizes.push_back(values.size());
          ++count;
        }
      }
      // Show sizes & capacities when increments occur
      std::cout << "Size/Capacity: ";
      for (size_t i{}; i < sizes.size(); ++i)
        std::cout << sizes.at(i) << "/" << capacities.at(i) << "  ";
      std::cout << std::endl;
      sizes.clear();      // Remove all elements
      capacities.clear(); // Remove all elements
    }
  }

} // namespace jizh03

#include <iostream> // For standard streams
#include <algorithm> // For copy()
#include <deque> // For deque container
#include <string> // For string classes
#include <iterator> // For front_insert_iterator & stream iterators
namespace jizh04
{
  // Ex2_04.cpp
  // Using a deque container

  using std::string;

  void test(void)
  {
    std::deque<string> names;
    std::cout << "Enter first names separated by spaces. Enter Ctrl+Z on a new line to end:\n";
    std::cout << std::endl;

    cin_clear();
    //std::cin.ignore(1024, '\n');
    //std::cin.sync();
    std::cin.ignore();
    std::cin.ignore();
    cin_clear();

    std::copy(std::istream_iterator<string>{std::cin},
              std::istream_iterator<string>{},
              std::front_inserter(names));
    std::cout << "\nIn reverse order, the names you entered are:\n";
    std::copy(std::begin(names), std::end(names), std::ostream_iterator<string>{std::cout, "  "});
    std::cout << std::endl;
  }
} // namespace jizh04

#include <iostream>
#include <list>
#include <string>
#include <functional>
namespace jizh05
{
  // Ex2_05.cpp
  // Working with a list

  using std::list;
  using std::string;

  // List a range of elements
  template <typename Iter>
  void list_elements(Iter begin, Iter end)
  {
    while (begin != end)
      std::cout << *begin++ << std::endl;
  }

  void test(void)
  {
    std::list<string> proverbs;

    // Read the proverbs
    std::cout << "Enter a few proverbs and enter an empty line to end:" << std::endl;
    string proverb;
    while (getline(std::cin, proverb, '\n'), !proverb.empty())
      proverbs.push_front(proverb);

    std::cout << "Go on, just one more:" << std::endl;
    getline(std::cin, proverb, '\n');
    proverbs.emplace_back(proverb);

    std::cout << "The elements in the list in reverse order are:" << std::endl;
    list_elements(std::rbegin(proverbs), std::rend(proverbs));

    proverbs.sort(); // Sort the proverbs in ascending sequence
    std::cout << "\nYour proverbs in ascending sequence are:" << std::endl;
    list_elements(std::begin(proverbs), std::end(proverbs));

    proverbs.sort(std::greater<>()); // Sort the proverbs in descending sequence
    std::cout << "\nYour proverbs in descending sequence:" << std::endl;
    list_elements(std::begin(proverbs), std::end(proverbs));
  }
} // namespace jizh05

int main(void)
{
  std::cout << "-----------" << std::endl;
  //jizh01::test();

  std::cout << "-----------" << std::endl;
  jizh02::test();

  std::cout << "-----------" << std::endl;
  //jizh03::test();

  std::cout << "-----------" << std::endl;
  jizh04::test();

  std::cout << "-----------" << std::endl;
  //jizh05::test();

  return 0;
}