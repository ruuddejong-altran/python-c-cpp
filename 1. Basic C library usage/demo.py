import ctypes
import platform

if platform.system() == 'Windows':
    libpath = './spamlib'
else:
    libpath = './libspamlib.so'

spam = ctypes.cdll.LoadLibrary(libpath)

# Signature of spam.add
spam.add.argtypes = [ctypes.c_int, ctypes.c_int]
spam.add.restype = ctypes.c_int

# Signature of spam.swap
spam.swap.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
spam.swap.restype = None

# Signature of the callback function used by spam.do_operation
# CFUNCTYPE arguments are: return type, arg1 type, arg2 type, ....
operator_functype = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int, ctypes.c_int)

# Signature of spam.do_operation
spam.do_operation.argtypes = [ctypes.c_int, ctypes.c_int, operator_functype]
spam.do_operation.restype = ctypes.c_int


def subtract(x, y):
    print(f"subtract is called with ({x}, {y})")
    return x - y


if __name__ == '__main__':
    x = 3
    y = 5
    print(f"x = {x}, y = {y}")

    # ctypes takes care of automatic conversion of Python ints to C ints
    result = spam.add(x, y)
    print(f"spam.add({x}, {y}) gives {result}")

    # Python does not support in-place assignment
    # Therefore we must explicitly use C integers,
    # and copy their Python values back to the original variables.
    _x = ctypes.c_int(x)
    _y = ctypes.c_int(y)
    spam.swap(ctypes.pointer(_x), ctypes.pointer(_y))
    x = _x.value
    y = _y.value
    print(f"After spam.swap(), x = {x}, y = {y}")

    # ctypes takes care of automatic conversion of Python ints to C ints
    # The callback function must be explicitly transformed to the
    # correct function signature that do_operation expects.
    result = spam.do_operation(x, y, operator_functype(subtract))
    print(f"do_operation({x}, {y}, subtract) gives {result}")
