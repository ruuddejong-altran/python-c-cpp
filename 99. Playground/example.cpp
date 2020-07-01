#include "playgroundlib.h"

int Dummy::max_serial_number_ = 0;

Dummy::Dummy() : serial_number_(++Dummy::max_serial_number_)
{
}

Dummy::~Dummy() = default;

int Dummy::id()
{
    return serial_number_;
}


std::unique_ptr<Dummy> MakeDummy()
{
    return std::make_unique<Dummy>();
}

ConstructVariations::ConstructVariations(std::function<std::unique_ptr<Dummy>()> func) :
        up_dummy_func_(std::move(func))
{
}

ConstructVariations::ConstructVariations(UP_Int_F func) : up_int_func_(func)
{
}

ConstructVariations::~ConstructVariations() = default;

std::unique_ptr<Dummy> ConstructVariations::CallUP_Dummy_func()
{
    return up_dummy_func_();
}

std::unique_ptr<int> ConstructVariations::CallUP_Int_func()
{
    return up_int_func_();
}

