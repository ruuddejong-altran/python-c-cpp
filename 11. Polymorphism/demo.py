from polymorphism import Base, ObjectRepresentation


if __name__ == '__main__':
    b = Base("Python 1")
    print(b.Repr())
    ObjectRepresentation(b)

    class PythonDerived(Base):
        def Repr(self):
            return f'<PythonDerived("{self.label}")>'

    d = PythonDerived("Python 2")
    print(d.Repr())
    ObjectRepresentation(d)
