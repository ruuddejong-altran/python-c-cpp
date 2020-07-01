#include <algorithm>
#include <chrono>
#include <map>

#include "traffic_light.h"

TrafficLight::TrafficLight(State initial_state) :
        current_state_(State::Off),
        state_change_cb_list_(),
        transition_sequence_(),
        transition_thread_(std::thread(&TrafficLight::TransitionRunner, this)),
        lights_mutex_(),
        transition_buffer_(),
        transition_mutex_(),
        transition_cv(),
        stop_signal_(),
        stop_transition_thread_(stop_signal_.get_future()),
        busy_(false)
{
    for (auto& name : light_names)
    {
        lights_.emplace_back(Light::MakeLight());
    }
    Init(initial_state);
}

TrafficLight::~TrafficLight()
{
    std::cout << "Destroying TrafficLight instance" << std::endl;
    stop_signal_.set_value();
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
    MoveTo(initial_state);
}

void TrafficLight::TransitToState(TrafficLight::State target_state)
{
    if (current_state_ != target_state)
    {
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
        RunTransition();
    }
}

void TrafficLight::PrepareTransition(State from_state, State target_state)
{
    transition_sequence_.clear();
    switch (target_state)
    {
        case State::Open:
            transition_sequence_.push_back(
                    {State::Open, {Light::State::Off, Light::State::Off, Light::State::On}, 3000});
            break;
        case State::Closed:
            transition_sequence_.push_back(
                    {State::Closing, {Light::State::Off, Light::State::On, Light::State::Off}, 2000});
            transition_sequence_.push_back(
                    {State::Closed, {Light::State::On, Light::State::Off, Light::State::Off}, 3000});
            break;
        case State::Warning:
            transition_sequence_.push_back(
                    {State::Warning, {Light::State::Off, Light::State::Flashing, Light::State::Off}, 3000});
            break;
        case State::Off:
            transition_sequence_.push_back(
                    {State::Off, {Light::State::Off, Light::State::Off, Light::State::Off}, 3000});
            break;
        default:
            break;
    }
}

void TrafficLight::TransitionRunner()
{
    State next_state;
    while (true)
    {
        using namespace std::chrono_literals;
        {
            std::unique_lock<std::mutex> lock(transition_mutex_);
            while (transition_buffer_.empty())
            {
                busy_ = false;
                transition_cv.wait_for(lock, 1ms);
                if (transition_buffer_.empty() && stop_transition_thread_.wait_for(1ms) != std::future_status::timeout)
                {
                    // thread has been asked to stop
                    return;
                }
            }
            busy_ = true;
            next_state = transition_buffer_.front();
            transition_buffer_.pop();
        }
        TransitToState(next_state);
    }
}

void TrafficLight::RunTransition()
{
    for (auto const&[state, pattern, delay_ms] : transition_sequence_)
    {
        current_state_ = state;
        SetLightPatternAndWait(pattern, delay_ms);
    }
}

void TrafficLight::SetLightPatternAndWait(TrafficLight::LightPattern pattern, int delay_ms)
{
    SetLightPattern(std::move(pattern));
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

void TrafficLight::SetLightPattern(TrafficLight::LightPattern pattern)
{
    const std::lock_guard<std::mutex> lock(lights_mutex_);
    auto value_iterator = pattern.begin();
    std::for_each(lights_.begin(), lights_.end(), [&](auto& light) { light->SetState(*value_iterator++); });
    for (auto& func : state_change_cb_list_)
    {
        RunCallbackFunction(func);
    }
}

std::vector<std::string> TrafficLight::GetLightNames()
{
    return light_names;
}

TrafficLight::LightPattern TrafficLight::GetLightPattern()
{
    TrafficLight::LightPattern result = TrafficLight::LightPattern();
    std::transform(lights_.begin(), lights_.end(), std::back_inserter(result),
            [](auto& light) -> Light::State { return light->GetState(); });
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

void TrafficLight::MoveTo(TrafficLight::State target_state)
{
    switch (target_state)
    {
        case State::Off:
        case State::Closed:
        case State::Open:
        case State::Warning:
            AddStateToTransitionBuffer(target_state);
            break;
        default:
            /* Unsupported target states */
            break;
    }
}

const std::vector<std::string> TrafficLight::light_names{"red", "amber", "green"};

void TrafficLight::AddStateToTransitionBuffer(TrafficLight::State state)
{
    std::lock_guard<std::mutex> lock(transition_mutex_);
    transition_buffer_.push(state);
    transition_cv.notify_all();
}

bool TrafficLight::InTransition()
{
    return busy_;
}

std::ostream& operator<<(std::ostream& out, TrafficLight::State state)
{
    static std::map<TrafficLight::State, std::string> stateStrings;
    if (stateStrings.empty())
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
