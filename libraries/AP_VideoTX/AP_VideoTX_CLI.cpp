#include "AP_VideoTX_CLI.h"
#include <AP_VideoTX/AP_VideoTX.h>
#include <AP_Logger/AP_Logger.h>

#if defined(HAL_VIDEOTX_CLI_ENABLED) && HAL_VIDEOTX_CLI_ENABLED

AP_VideoTX_CLI* AP_VideoTX_CLI::_singleton;

// Параметры для одной команды
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

    // @Param: POWER
    // @DisplayName: VTX Power
    // @Description: VTX power (0=no change, 1=25mW, 2=100mW, 3=200mW, 4=400mW, 5=600mW)
    // @Values: 0:NoChange,1:25mW,2:100mW,3:200mW,4:400mW,5:600mW
    // @User: Standard
    AP_GROUPINFO("POWER", 6, VTX_CLI_Command, power, 0),

    AP_GROUPEND
};

// Главные параметры с subgroups
const AP_Param::GroupInfo AP_VideoTX_CLI::var_info[] = {
    // @Group: 0_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[0], "0_", 1, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 1_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[1], "1_", 2, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 2_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[2], "2_", 3, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 3_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[3], "3_", 4, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 4_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[4], "4_", 5, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 5_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[5], "5_", 6, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 6_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[6], "6_", 7, AP_VideoTX_CLI, VTX_CLI_Command),

    // @Group: 7_
    // @Path: AP_VideoTX_CLI.cpp
    AP_SUBGROUPINFO(commands[7], "7_", 8, AP_VideoTX_CLI, VTX_CLI_Command),

    AP_GROUPEND
};

AP_VideoTX_CLI::AP_VideoTX_CLI()
    : _initialized(false)
    , _last_update_ms(0)
    , _last_active_idx(-1)
{
    if (_singleton != nullptr) {
        AP_HAL::panic("AP_VideoTX_CLI must be singleton");
    }
    _singleton = this;

    AP_Param::setup_object_defaults(this, var_info);
}

void AP_VideoTX_CLI::init()
{
    if (_initialized) {
        return;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < VTX_CLI_COMMANDS_MAX; i++) {
        if (commands[i].is_configured()) {
            count++;
            if (commands[i].pwm_begin > commands[i].pwm_end) {
                GCS_SEND_TEXT(MAV_SEVERITY_WARNING, "VTXCLI%u: BEGIN > END", i);
            }
        }
    }

    _initialized = true;
    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX CLI: %u commands", count);
}

void AP_VideoTX_CLI::update()
{
    if (!_initialized) {
        return;
    }

    const uint32_t now_ms = AP_HAL::millis();
    if (now_ms - _last_update_ms < UPDATE_INTERVAL_MS) {
        return;
    }
    _last_update_ms = now_ms;

    int8_t active_idx = -1;
    const VTX_CLI_Command* active_cmd = nullptr;

    for (uint8_t i = 0; i < VTX_CLI_COMMANDS_MAX; i++) {
        if (commands[i].is_configured() && commands[i].is_active()) {
            active_idx = i;
            active_cmd = &commands[i];
            break;
        }
    }

    if (active_idx != _last_active_idx) {
        if (active_cmd != nullptr) {
            apply_command(*active_cmd);
            send_status_msg(*active_cmd, active_idx);
        }
        _last_active_idx = active_idx;
    }
}

void AP_VideoTX_CLI::apply_command(const VTX_CLI_Command& cmd)
{
    AP_VideoTX* vtx = AP_VideoTX::get_singleton();
    if (vtx == nullptr) {
        return;
    }

    bool changed = false;

    if (cmd.band > 0 && cmd.band <= 5) {
        vtx->set_band(cmd.band - 1);
        changed = true;
    }

    if (cmd.channel > 0 && cmd.channel <= 8) {
        vtx->set_channel(cmd.channel - 1);
        changed = true;
    }

    if (cmd.power > 0) {
        vtx->set_power_level(cmd.power - 1);
        changed = true;
    }

    if (changed) {
        vtx->set_options(vtx->get_options() & ~uint8_t(AP_VideoTX::VideoOptions::VTX_PITMODE));
        
#if HAL_LOGGING_ENABLED
        AP::logger().Write("VTXC", "TimeUS,Cmd,Band,Chan,Pwr", "QBBBB",
                          AP_HAL::micros64(),
                          (uint8_t)(_last_active_idx + 1),
                          (uint8_t)cmd.band.get(),
                          (uint8_t)cmd.channel.get(),
                          (uint8_t)cmd.power.get());
#endif
    }
}

void AP_VideoTX_CLI::send_status_msg(const VTX_CLI_Command& cmd, uint8_t idx)
{
    static const uint16_t vtx_freq[5][8] = {
        {5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},
        {5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},
        {5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},
        {5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},
        {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},
    };

    uint16_t freq = 0;
    if (cmd.band > 0 && cmd.band <= 5 && cmd.channel > 0 && cmd.channel <= 8) {
        freq = vtx_freq[cmd.band - 1][cmd.channel - 1];
    }

    GCS_SEND_TEXT(MAV_SEVERITY_INFO, "VTX CMD%u: B%u/C%u P%u (%uMHz)",
                  idx,
                  (unsigned)cmd.band.get(),
                  (unsigned)cmd.channel.get(),
                  (unsigned)cmd.power.get(),
                  freq);
}

const VTX_CLI_Command* AP_VideoTX_CLI::get_command(uint8_t index) const
{
    if (index >= VTX_CLI_COMMANDS_MAX) {
        return nullptr;
    }
    return &commands[index];
}

const VTX_CLI_Command* AP_VideoTX_CLI::get_active_command() const
{
    if (_last_active_idx < 0) {
        return nullptr;
    }
    return &commands[_last_active_idx];
}

namespace AP {
    AP_VideoTX_CLI* videotx_cli() {
        return AP_VideoTX_CLI::get_singleton();
    }
}

#endif