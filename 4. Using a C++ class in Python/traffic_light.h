#ifndef PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H
#define PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "light.h"

class TrafficLight
{
public:
    using LightPattern = std::vector<Light::State>;
    using CallbackFunction = std::function<void(TrafficLight*)>;
    using LightFactory = std::function<std::shared_ptr<Light>()>;
    static const std::vector<std::string> light_names;

    enum class State {Off, Closing, Closed, Opening, Open, Warning};

    explicit TrafficLight(State initial_state = State::Off);
    virtual ~TrafficLight();
    virtual State GetState();
    virtual std::vector<std::string> GetLightNames();
    virtual LightPattern GetLightPattern();
    virtual void MoveTo(State target_state);
    virtual void AddCallback(const CallbackFunction& func);

protected:
    virtual void PrepareTransition(State from_state, State target_state);

private:
    using TransitionElement = std::tuple<State, LightPattern, int>;
    using TransitionSequence = std::vector<TransitionElement>;

    void Init(State initial_state);
    void RunCallbackFunction(const CallbackFunction& func);
    void RunTransition();
    void SetLightPattern(LightPattern pattern);
    void SetLightPatternAndWait(LightPattern pattern, int delay_ms);
    void TransitToState(State target_state);
    void AddStateToTransitionBuffer(State state);
    void TransitionRunner();

    State current_state_;
    std::vector<std::unique_ptr<Light>> lights_;
    std::vector<CallbackFunction> state_change_cb_list_;
    TransitionSequence transition_sequence_;
    std::thread transition_thread_;
    std::mutex lights_mutex_;
    std::queue<State> transition_buffer_;
    std::mutex transition_mutex_;
    std::condition_variable transition_cv;
    std::promise<void> stop_signal_;
    std::future<void> stop_transition_thread_;

};

std::ostream& operator<<(std::ostream& out, TrafficLight::State state);

#endif //PYTHON_C_C_EXAMPLE_4_TRAFFIC_LIGHT_H
