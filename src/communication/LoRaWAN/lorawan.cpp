/**
 * @file    lorawan.cpp
 * @brief   LMIC callbacks (credentials and events) for the LoRaWAN module.
 *          They must be defined in a single translation unit, because LMIC
 *          looks them up by name at link time.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#include "lorawan.hpp"

extern "C"
{
    void os_getDevEui(u1_t* buf)
    {
        for (int i = 0; i < 8; i++)
            buf[i] = lorawanPtr->devEui[7 - i];
    }


    void os_getArtEui(u1_t* buf)
    {
        for (int i = 0; i < 8; i++)
            buf[i] = lorawanPtr->appEui[7 - i];
    }


    void os_getDevKey(u1_t* buf)
    {
        memcpy(buf, lorawanPtr->appKey, 16);
    }


    void onEvent(ev_t event)
    {
        lorawanPtr->handleEvent(event);
    }
}