#ifndef PYTHON_C_C_PLAYGROUND_PLAYGROUNDLIB_H
#define PYTHON_C_C_PLAYGROUND_PLAYGROUNDLIB_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>

class Dummy
{
public:
    Dummy();
    virtual ~Dummy();
    int id();
private:
    int serial_number_;
    static int max_serial_number_;
};


using UP_Dummy_F = std::function<std::unique_ptr<Dummy>()>;
using UP_Int_F = std::function<std::unique_ptr<int>()>;

std::unique_ptr<Dummy> MakeDummy();

class ConstructVariations
{
public:
    explicit ConstructVariations(UP_Dummy_F func);
    explicit ConstructVariations(UP_Int_F func);
    virtual ~ConstructVariations();
    std::unique_ptr<Dummy> CallUP_Dummy_func();
    std::unique_ptr<int> CallUP_Int_func();
private:
    std::function<std::unique_ptr<Dummy>()> up_dummy_func_;
    std::function<std::unique_ptr<int>()> up_int_func_;
};

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

#endif //PYTHON_C_C_PLAYGROUND_PLAYGROUNDLIB_H
