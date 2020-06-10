#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include "light.h"
#include "traffic_light.h"

//void show_traffic_light(std::shared_ptr<TrafficLight> traffic_light)
//{
//    auto state = traffic_light->GetState();
//    std::cout << "State: " << state << " (";
//    auto lights = traffic_light->GetLights();
//    auto names = TrafficLight::light_names;
//    auto light_iterator = lights.begin();
//    auto name_iterator = names.begin();
//    size_t n = names.size();
//    while (light_iterator != lights.end() && name_iterator != names.end())
//    {
//        std::cout << *name_iterator++ << ": " << (*light_iterator++)->GetState();
//        if (--n > 0)
//        {
//            std::cout << ", ";
//        }
//        else
//        {
//            std::cout << ")" << std::endl;
//        }
//    }
//}

void monitor(TrafficLight* traffic_light)
{
    std::cout << "State: " << traffic_light->GetState() << " (";
    auto names = traffic_light->GetLightNames();
    auto name_iterator = names.begin();
    auto pattern = traffic_light->GetLightPattern();
    auto pattern_iterator = pattern.begin();
    size_t n = names.size();
    while (pattern_iterator != pattern.end() && name_iterator != names.end())
    {
        std::cout << *name_iterator++ << ": " << *pattern_iterator++;
        if (--n > 0)
        {
            std::cout << ", ";
        }
        else
        {
            std::cout << ")" << std::endl;
        }
    }
}

int main()
{
    std::cout << "Testing Light class" << std::endl;
    std::shared_ptr<Light> light = std::make_shared<Light>();
    std::cout << "Light initialized with " << light->GetState() << std::endl;
    light->SetState(Light::State::On);
    std::cout << "Light changed to " << light->GetState() << std::endl;

    std::cout << "-----------" << std::endl;
    std::cout << "Testing traffic light" << std::endl;

    auto make_light = []() { return std::make_shared<Light>(); };
    auto traffic_light = std::make_shared<TrafficLight>(make_light);
    traffic_light->AddCallback(monitor);
    std::cout << "Denying traffic" << std::endl;
    traffic_light->Close();
    std::cout << "Allowing traffic" << std::endl;
    traffic_light->Open();
    std::cout << "Closing again" << std::endl;
    traffic_light->Close();
    std::cout << "Warning" << std::endl;
    traffic_light->Warning();
}
