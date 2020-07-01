#ifndef PYTHON_C_C_CONVERSION_EXAMPLE_H
#define PYTHON_C_C_CONVERSION_EXAMPLE_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>

template<typename T>
void print_sequence(std::vector<T> seq)
{
    std::cout << "[";
    if (!seq.empty())
    {
        std::cout << seq[0];
        std::for_each(++seq.begin(), seq.end(),
                      [](const auto& value)
                      {
                          std::cout << ", " << value;
                      });
    }
    std::cout << "]" << std::endl;
}

template <typename T>
void add_to_sequence(std::vector<T>& seq, T value)
{
    std::cout << "Before: ";
    print_sequence(seq);
    seq.push_back(value);
    std::cout << "After: ";
    print_sequence(seq);
}

std::vector<int> global_list {10, 11, 12};

void print_global_list()
{
    print_sequence(global_list);
}

#endif //PYTHON_C_C_CONVERSION_EXAMPLE_H
