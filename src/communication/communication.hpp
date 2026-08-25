#pragma once

#include <stdint.h>
#include <stddef.h>

class Communication
{
    protected:

        bool initialized = false;
        static inline int communicationCount;

    public:

        Communication();
        virtual bool begin() = 0;
        virtual bool send(uint8_t* data, size_t size) = 0;
        bool isInitialized() const;
        static int getCommunicationCount();
        
        virtual ~Communication() = default;
};

inline Communication::Communication()
    : initialized(false)
{
    Communication::communicationCount++;
}

inline bool Communication::isInitialized() const
{
    return initialized;
}

inline int Communication::getCommunicationCount()
{
    return communicationCount;
}