/**
 * @file    main.cpp
 * @brief   Entry point of the HomeStation firmware.
 * @author  Lucas Pichon
 * @date    2026-07-16
 */

#include "app/station.hpp"

Station station;

void setup() { station.setup(); }
void loop()  { station.loop();  }