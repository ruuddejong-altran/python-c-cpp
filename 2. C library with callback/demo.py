import ctypes


def py_greeting_callback(msg):
    print(msg.decode('utf8'))


CFUNC_GREETING_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_char_p)

greeting_callback = CFUNC_GREETING_CALLBACK(py_greeting_callback)

greeting = ctypes.cdll.LoadLibrary('cmake-build-release/greeting')

if __name__ == '__main__':
    greeting.greeting(greeting_callback, 'Hi from Python'.encode('utf-8'))
