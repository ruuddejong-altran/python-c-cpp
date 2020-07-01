#include <iostream>
#include <algorithm>

#include "playgroundlib.h"

static std::unique_ptr<int> make_int_ptr()
{
    return std::make_unique<int>();
}

int main()
{
    auto dummy1 = Dummy();
    std::cout << "Dummy 1 has id: " << dummy1.id() << std::endl;

    auto dummy2 = MakeDummy();
    std::cout << "Dummy 2 has id: " << dummy2->id () << std::endl;

    /* Constructor with function that returns unique_ptr to Dummy */
    auto c_up_dummy_f = ConstructVariations(MakeDummy);
    auto dummy_3 = c_up_dummy_f.CallUP_Dummy_func();
    std::cout << "Dummy 3 has id: " << dummy_3->id() << std::endl;

    /* Constructor with function that returns unique_ptr to int */
    auto c_up_int_f = ConstructVariations(make_int_ptr);
    auto dummy_int = c_up_int_f.CallUP_Int_func();
    std::cout << "Dummy int: " << *dummy_int << ", " << ++(*dummy_int) << std::endl;

    /* In-place extension of sequence */
    std::vector<int> int_seq {0, 1, 2};
    add_to_sequence(int_seq, 4);
    std::cout << "In main: ";
    print_sequence(int_seq);
    add_to_sequence(global_list, 5);
    print_global_list();

    return 0;
}
