// AP_VideoTX_CLI.cpp
#include "AP_VideoTX_CLI.h"

const AP_Param::GroupInfo VTX_CLI_Command::var_info[] = {
    // @Param: RC
    // @DisplayName: RC Channel
    // @Description: RC channel to monitor (-1=disabled, 0-15=channel)
    // @Range: -1 15
    // @User: Standard
    AP_GROUPINFO("RC", 1, VTX_CLI_Command, rc_channel, -1),

    // @Param: BEGIN
    // @DisplayName: PWM Begin
    // @Description: Minimum PWM value to activate
    // @Range: 800 2200
    // @Units: PWM
    // @User: Standard
    AP_GROUPINFO("BEGIN", 2, VTX_CLI_Command, pwm_begin, 0),

    // @Param: END
    // @DisplayName: PWM End
    // @Description: Maximum PWM value to activate
    // @Range: 800 2200
    // @Units: PWM
    // @User: Standard
    AP_GROUPINFO("END", 3, VTX_CLI_Command, pwm_end, 0),

    // @Param: BAND
    // @DisplayName: VTX Band
    // @Description: VTX band (0=no change, 1=A, 2=B, 3=E, 4=F, 5=R)
    // @Values: 0:NoChange,1:BandA,2:BandB,3:BandE,4:BandF,5:Raceband
    // @User: Standard
    AP_GROUPINFO("BAND", 4, VTX_CLI_Command, band, 0),

    // @Param: CHAN
    // @DisplayName: VTX Channel
    // @Description: VTX channel (0=no change, 1-8)
    // @Range: 0 8
    // @User: Standard
    AP_GROUPINFO("CHAN", 5, VTX_CLI_Command, channel, 0),

    // @Param: PWR
    // @DisplayName: VTX Power
    // @Description: VTX power level (0=no change, 1-5)
    // @Values: 0:NoChange,1:25mW,2:100mW,3:200mW,4:400mW,5:600mW
    // @User: Standard
    AP_GROUPINFO("PWR", 6, VTX_CLI_Command, power, 0),

    AP_GROUPEND
};

const AP_Param::GroupInfo AP_VideoTX_CLI::var_info[] = {
    // @Group: 0_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[0], "V0_", 1, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 1_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[1], "V1_", 2, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 2_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[2], "V2_", 3, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 3_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[3], "V3_", 4, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 4_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[4], "V4_", 5, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 5_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[5], "V5_", 6, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 6_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[6], "V6_", 7, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 7_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[7], "V7_", 8, AP_VideoTX_CLI, VTX_CLI_Command),

    AP_GROUPEND
};

AP_VideoTX_CLI *AP_VideoTX_CLI::_singleton;

AP_VideoTX_CLI::AP_VideoTX_CLI()
{
    if (_singleton) {
        return;
    }
    _singleton = this;
    AP_Param::setup_object_defaults(this, var_info);
}

AP_VideoTX_CLI *AP_VideoTX_CLI::get_singleton()
{
    return _singleton;
}

const VTX_CLI_Command* AP_VideoTX_CLI::get_active_command() const
{
    for (uint8_t i = 0; i < VTX_CLI_NUM_COMMANDS; i++) {
        if (commands[i].is_configured() && commands[i].is_active()) {
            return &commands[i];
        }
    }
    return nullptr;
}

namespace AP {
    AP_VideoTX_CLI *vtx_cli()
    {
        return AP_VideoTX_CLI::get_singleton();
    }
};