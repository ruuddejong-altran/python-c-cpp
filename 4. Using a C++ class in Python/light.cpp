#include <map>

#include "light.h"

Light::Light(Light::State state) : state_(state)
{
}

Light::State Light::GetState() const
{
    return state_;
}

void Light::SetState(Light::State state)
{
    state_ = state;
}

std::ostream& operator<<(std::ostream& out, const Light::State state)
{
    static std::map<Light::State, std::string> stateStrings;
    if (stateStrings.size() == 0)
    {
        auto extractEnumSymbol = [](const std::string& s)
        {
            std::size_t last_colon = s.rfind(':');
            return (last_colon == std::string::npos) ? s : s.substr(last_colon + 1);
        };
#define ADD_ENUM_STRING(x) stateStrings[x] = extractEnumSymbol(std::string(#x))
        ADD_ENUM_STRING(Light::State::Off);
        ADD_ENUM_STRING(Light::State::On);
        ADD_ENUM_STRING(Light::State::Flashing);
#undef ADD_ENUM_STRING
    }
    return out << stateStrings[state];
}
