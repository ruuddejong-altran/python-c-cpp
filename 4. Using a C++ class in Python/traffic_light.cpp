#include <chrono>
#include <condition_variable>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <thread>
#include "light.h"
#include "traffic_light.h"


TrafficLight::TrafficLight(const std::function<std::shared_ptr<Light>()>& light_factory,
                           TrafficLight::State initial_state) :
        current_state_(State::Off),
        state_change_cb_list_(),
        transition_sequence_(),
        transition_thread_(),
        lights_mutex_(),
        transition_mutex_()
{
    for (auto& name : light_names)
    {
        auto light = light_factory();
        lights_.push_back(light);
        light_map_[name] = light;
    }
    Init(initial_state);
}

TrafficLight::~TrafficLight()
{
    if (transition_thread_.joinable())
    {
        transition_thread_.join();
    }
}

TrafficLight::TrafficLight::State TrafficLight::GetState()
{
    return current_state_;
}

void TrafficLight::Init(TrafficLight::State initial_state)
{
    SetState(initial_state);
}

void TrafficLight::SetState(TrafficLight::State target_state)
{
    if (current_state_ != target_state)
    {
        if (transition_thread_.joinable())
        {
            transition_thread_.join();
        }
        State from_state = current_state_;
        switch (target_state)
        {
            case State::Open:
                current_state_ = State::Opening;
                break;
            case State::Closed:
                current_state_ = State::Closing;
                break;
            default:
                break;
        }
        PrepareTransition(from_state, target_state);
        transition_thread_ = std::thread(&TrafficLight::RunTransition, this, target_state);
    }
}

void TrafficLight::PrepareTransition(State /*from_state*/, State target_state)
{
    transition_sequence_.clear();
    switch (target_state)
    {
        case State::Open:
            transition_sequence_.push_back({{LS::Off, LS::Off, LS::On}, 3000});
            break;
        case State::Closed:
            transition_sequence_.push_back({{LS::Off, LS::On, LS::Off}, 2000});
            transition_sequence_.push_back({{LS::On, LS::Off, LS::Off}, 3000});
            break;
        case State::Warning:
            transition_sequence_.push_back({{LS::Off, LS::Flashing, LS::Off}, 3000});
            break;
        case State::Off:
            transition_sequence_.push_back({{LS::Off, LS::Off, LS::Off}, 3000});
            break;
        default:
            break;
    }
}

void TrafficLight::RunTransition(State to_state)
{
    const std::lock_guard<std::mutex> lock(transition_mutex_);
    size_t n = transition_sequence_.size();
    for (auto const&[pattern, delay_ms] : transition_sequence_)
    {
        if (--n == 0)
        {
            // Final transition pattern. Set current state to target state.
            current_state_ = to_state;
        }
        SetLightPatternAndWait(pattern, delay_ms);
    }
}

void TrafficLight::SetLightPatternAndWait(TrafficLight::LightPattern pattern, int delay_ms)
{
    SetLightPattern(pattern);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

void TrafficLight::SetLightPattern(TrafficLight::LightPattern pattern)
{
    const std::lock_guard<std::mutex> lock(lights_mutex_);

    auto light_iterator = lights_.begin();
    auto value_iterator = pattern.begin();
    while ((light_iterator != lights_.end()) && (value_iterator != pattern.end()))
    {
        auto light = *light_iterator++;
        auto value = *value_iterator++;
        light->SetState(value);
    }
    for (auto& func : state_change_cb_list_)
    {
        RunCallbackFunction(func);
    }
}

TrafficLight::LightNames TrafficLight::GetLightNames()
{
    return light_names;
}

TrafficLight::LightPattern TrafficLight::GetLightPattern()
{
    TrafficLight::LightPattern result = TrafficLight::LightPattern();
    auto result_iterator = result.begin();
    auto light_iterator = lights_.begin();
    while ((result_iterator != result.end()) && (light_iterator != lights_.end()))
    {
        *result_iterator++ = (*light_iterator++)->GetState();
    }
    return result;
}

void TrafficLight::AddCallback(const TrafficLight::CallbackFunction& func)
{
    state_change_cb_list_.push_back(func);
}

void TrafficLight::RunCallbackFunction(const TrafficLight::CallbackFunction& func)
{
    func(this);
}

void TrafficLight::Open()
{
    SetState(State::Open);
}

void TrafficLight::Close()
{
    SetState(State::Closed);
}

void TrafficLight::Warning()
{
    SetState(State::Warning);
}

void TrafficLight::Off()
{
    SetState(State::Off);
}

std::ostream& operator<<(std::ostream& out, TrafficLight::State state)
{
    static std::map<TrafficLight::State, std::string> stateStrings;
    if (stateStrings.size() == 0)
    {
        auto extractEnumSymbol = [](const std::string& s)
        {
            std::size_t last_colon = s.rfind(':');
            return (last_colon == std::string::npos) ? s : s.substr(last_colon + 1);
        };
#define ADD_ENUM_STRING(x) stateStrings[x] = extractEnumSymbol(std::string(#x))
        ADD_ENUM_STRING(TrafficLight::State::Off);
        ADD_ENUM_STRING(TrafficLight::State::Closed);
        ADD_ENUM_STRING(TrafficLight::State::Closing);
        ADD_ENUM_STRING(TrafficLight::State::Open);
        ADD_ENUM_STRING(TrafficLight::State::Opening);
        ADD_ENUM_STRING(TrafficLight::State::Warning);
#undef ADD_ENUM_STRING
    }
    return out << stateStrings[state];
}
