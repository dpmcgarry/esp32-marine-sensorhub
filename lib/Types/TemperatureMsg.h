#pragma once

#include <string>
#include <nlohmann/json.hpp>

struct TemperatureMessage
{
    std::string MAC;
    float TempC;
    float TempF;
    float BatteryPercent;
};

inline void to_json(nlohmann::json& j, const TemperatureMessage& t)
{
    j = nlohmann::json{ {"MAC", t.MAC}, {"TempC", t.TempC}, {"TempF", t.TempF}, {"BatteryPct", t.BatteryPercent} };
}