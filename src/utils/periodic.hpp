/**
 * @file    periodic.hpp
 * @brief   Utility structure for handling periodic tasks.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

struct Periodic
{   
    unsigned long period = 0;
    unsigned long last   = 0;

    bool due(unsigned long now)
    {
        if (now - last < period) 
        {
            return false;
        }
        last = now;
        return true;
    }
};