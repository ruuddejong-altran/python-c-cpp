import ctypes

greeting = ctypes.cdll.LoadLibrary('cmake-build-debug/greeting')

if __name__ == '__main__':
    message = 'Greetings from Python'
    greeting.greeting(message.encode('utf8'))  # Python3 strings are Unicode, C expects bytes.
