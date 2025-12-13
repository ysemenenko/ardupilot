#include "AP_YCustom.h"

#include <AP_Param/AP_Param.h>

extern const AP_HAL::HAL& hal;

// Параметры для каждого экземпляра (V1, V2, V3, V4)
const AP_Param::GroupInfo AP_YCustom::Params::var_info[] = {
    // @Param: DEBUG
    // @DisplayName: Debug level
    // @Description: Set to non-zero to enable debug messages.
    // @Values: 0:Disabled,2:ShowSlips,3:ShowOverruns
    // @User: Advanced
    AP_GROUPINFO("DEBUG", 0, AP_YCustom::Params, debug, 0),

    // @Param: LOOP_RATE
    // @DisplayName: Main loop rate
    // @Description: This controls the rate of the main control loop in Hz.
    // @Range: 50 400
    // @RebootRequired: True
    // @Units: Hz
    // @User: Advanced
    AP_GROUPINFO("LOOP_RATE", 1, AP_YCustom::Params, loop_rate_hz, DEFAULT_LOOP_RATE),

    AP_GROUPEND
};

// Главная таблица параметров
const AP_Param::GroupInfo AP_YCustom::var_info[] = {
    // @Group: V1_
    // @Path: AP_YCustom.cpp
    AP_SUBGROUPINFO(params[0], "V1_", 0, AP_YCustom, AP_YCustom::Params),

    // @Group: V2_
    // @Path: AP_YCustom.cpp
    AP_SUBGROUPINFO(params[1], "V2_", 1, AP_YCustom, AP_YCustom::Params),

    // @Group: V3_
    // @Path: AP_YCustom.cpp
    AP_SUBGROUPINFO(params[2], "V3_", 2, AP_YCustom, AP_YCustom::Params),

    // @Group: V4_
    // @Path: AP_YCustom.cpp
    AP_SUBGROUPINFO(params[3], "V4_", 3, AP_YCustom, AP_YCustom::Params),

    AP_GROUPEND
};

// constructor
AP_YCustom::AP_YCustom()
{
    if (_singleton) {
        return;
    }
    _singleton = this;

    AP_Param::setup_object_defaults(this, var_info);
}

AP_YCustom *AP_YCustom::_singleton;
AP_YCustom *AP_YCustom::get_singleton()
{
    return _singleton;
}

namespace AP {

AP_YCustom &ycustom()
{
    return *AP_YCustom::get_singleton();
}

};