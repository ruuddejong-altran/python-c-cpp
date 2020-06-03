import spam


def subtract(x, y):
    print(f"subtract is called with ({x}, {y})")
    return x - y


if __name__ == '__main__':
    x = 3
    y = 5
    print(f"x = {x}, y = {y}")

    result = spam.add(x, y)
    print(f"spam.add({x}, {y}) gives {result}")

    x, y = spam.swap(x, y)
    print(f"After spam.swap(), x = {x}, y = {y}")

    result = spam.do_operation(x, y, subtract)
    print(f"do_operation({x}, {y}, subtract) gives {result}")
