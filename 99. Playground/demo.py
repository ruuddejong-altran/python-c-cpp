from playground import Dummy, MakeDummy


if __name__ == '__main__':
    dummy = Dummy()
    print(f"dummy (type {type(dummy)}) has id {dummy.id}")

    dummy2 = MakeDummy()
    print(f"dummy2 (type {type(dummy2)}) has id {dummy2.id}")

    # cwupf1 = ConstructWithUniquePointerFunction()
    # dummy3 = cwupf1.CallFunc()
    # print(f"dummy3 (type {type(dummy3)}) has id {dummy3.id}")



