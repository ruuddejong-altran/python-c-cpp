import ctypes
import platform

if platform.system() == 'Windows':
    _libpath = './spamlib'
else:
    _libpath = './libspamlib.so'

_spam = ctypes.cdll.LoadLibrary(_libpath)

# Signature of spam.add
_spam.add.argtypes = [ctypes.c_int, ctypes.c_int]
_spam.add.restype = ctypes.c_int

# Signature of spam.swap
_spam.swap.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)]
_spam.swap.restype = None

# Signature of the callback function used by spam.do_operation
# CFUNCTYPE parameters are: return type, arg1 type, arg2 type, ....
_operation_functype = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int, ctypes.c_int)

# Signature of spam.do_operation
_spam.do_operation.argtypes = [ctypes.c_int, ctypes.c_int, _operation_functype]
_spam.do_operation.restype = ctypes.c_int


def add(x, y):
    return _spam.add(x, y)


def swap(x, y):
    _x = ctypes.c_int(x)
    _y = ctypes.c_int(y)
    _spam.swap(ctypes.pointer(_x), ctypes.pointer(_y))
    return _x.value, _y.value


def do_operation(x, y, operator):
    return _spam.do_operation(x, y, _operation_functype(operator))