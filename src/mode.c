#include "mode.h"

const char* getModeName(Mode mode) {
    
    switch(mode) 
    {
        case MODE_WALK: return "Walk";
        case MODE_METRO: return "Metro";
        case MODE_CAR: return "Car";
        case MODE_BIKOLPO: return "Bikolpo Bus";
        case MODE_UTTARA: return "Uttara Bus";
        default: return "Unknown";
    }
}