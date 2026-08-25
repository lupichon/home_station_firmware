/**
 * @file    gas_state.hpp
 * @brief   Classifies a raw gas sensor reading into qualitative levels and
 *          provides string conversion for display/logging purposes.
 * @author  Lucas Pichon
 * @date    2026-30-07
 */

#pragma once

#include <stdint.h>

// ============================================================
// GasState enum definition
// ============================================================

/**
 * @brief Qualitative gas level derived from a raw sensor reading.
 */
enum class GasState : uint8_t
{
    Good     = 0,   // Raw reading below the "moderate" threshold
    Moderate = 1,   // Raw reading between the "good" and "high" thresholds
    High     = 2,   // Raw reading between the "moderate" and "danger" thresholds
    Danger   = 3,   // Raw reading at or above the "danger" threshold
};

// ============================================================
// Conversion helpers
// ============================================================

/**
 * @brief Classify a raw gas sensor reading into a GasState level.
 * @param raw Raw gas sensor reading.
 * @return Corresponding GasState:
 *         - Good     if raw < 500
 *         - Moderate if raw < 1500
 *         - High     if raw < 2500
 *         - Danger   otherwise
 */
inline GasState gasStateFromRaw(uint16_t raw)
{
    if (raw < 500)  return GasState::Good;
    if (raw < 1500) return GasState::Moderate;
    if (raw < 2500) return GasState::High;
    return GasState::Danger;
}

/**
 * @brief Convert a GasState value to a human-readable string.
 * @param state GasState value to convert.
 * @return String representation of the state (e.g. "Good", "Danger"),
 *         or "Unknown" if the value doesn't match any known state.
 */
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