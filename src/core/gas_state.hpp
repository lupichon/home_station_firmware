#pragma once

#include <stdint.h>

enum class GasState : uint8_t
{
    Good     = 0,
    Moderate = 1,
    High     = 2,
    Danger   = 3,
};

inline GasState gasStateFromRaw(uint16_t raw)
{
    if (raw < 500)  return GasState::Good;
    if (raw < 1500) return GasState::Moderate;
    if (raw < 2500) return GasState::High;
    return GasState::Danger;
}

inline const char* gasStateToString(GasState state)
{
    switch (state)
    {
        case GasState::Good:     return "Good";
        case GasState::Moderate: return "Moderate";
        case GasState::High:     return "High";
        case GasState::Danger:   return "Danger";
        default:                 return "Unknown";
    }
}