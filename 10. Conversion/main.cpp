#include <iostream>

#include "example.h"

int main()
{
    /* In-place extension of sequence */
    std::vector<int> int_seq {0, 1, 2};
    add_to_sequence(int_seq, 4);
    std::cout << "In main: ";
    print_sequence(int_seq);

    std::cout << "Global list: ";
    print_global_list();
    add_to_sequence(global_list, 5);
    std::cout << "Appended 5 to global list: ";
    print_global_list();

    return 0;
}
