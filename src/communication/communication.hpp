#pragma once

#include <stdint.h>
#include <stddef.h>

class Communication
{
    protected:

        bool initialized = false;

    public:

        Communication();
        virtual bool begin() = 0;
        virtual bool send(uint8_t* data, size_t size) = 0;
        bool isInitialized() const;
        
        virtual ~Communication() = default;
};

Communication::Communication()
    : initialized(false)
{
    
}

inline bool Communication::isInitialized() const
{
    return initialized;
}