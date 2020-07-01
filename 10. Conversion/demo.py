from conversion import VectorInt, add_to_sequence, global_list, print_global_list


if __name__ == '__main__':
    x = VectorInt([1, 2, 3])
    print(f"x = {x}")
    print("Calling 'add_to_sequence(x, 4)'")
    add_to_sequence(x, 4)
    print(f"x = {x}")

    print(f"global_list = {global_list}")
    print("Appending 7 to global list")
    global_list.append(7)
    print("Printed from C++:")
    print_global_list()
