#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include "light.h"
#include "traffic_light.h"


void monitor(TrafficLight* traffic_light)
{
    auto names = traffic_light->GetLightNames();
    auto pattern = traffic_light->GetLightPattern();
    auto pattern_iterator = pattern.begin();
    std::cout << "State: " << traffic_light->GetState() << " (";
    std::for_each(names.begin(), names.end(),
                  [&](const auto& name)
                  {
                      std::cout << name << ": " << *pattern_iterator++;
                      if (pattern_iterator != pattern.end())
                      {
                          std::cout << ", ";
                      }
                  });
    std::cout << ")" << std::endl;
}

using std::to_string;

int main()
{
    std::cout << "Testing Light class" << std::endl;
    std::shared_ptr<Light> light = std::make_shared<Light>();
    std::cout << "Light initialized with " << light->GetState() << std::endl;
    light->SetState(Light::State::On);
    std::ostringstream ss;
    ss << light->GetState();
    std::string light_state = ss.str();
    std::cout << "Light changed to " << light_state << std::endl;

    std::cout << "-----------" << std::endl;
    std::cout << "Testing traffic light" << std::endl;

    constexpr auto Off = TrafficLight::State::Off;
    constexpr auto Closed = TrafficLight::State::Closed;
    constexpr auto Open = TrafficLight::State::Open;
    constexpr auto Warning = TrafficLight::State::Warning;

    auto traffic_light = std::make_shared<TrafficLight>();
    traffic_light->AddCallback(monitor);
    traffic_light->MoveTo(Closed);
    traffic_light->MoveTo(Open);
    traffic_light->MoveTo(Closed);
    traffic_light->MoveTo(Warning);
    traffic_light->MoveTo(Off);
}
