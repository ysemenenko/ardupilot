#include "AP_VideoTX_CLI.h"
#include <AP_VideoTX/AP_VideoTX.h>
#include <AP_Logger/AP_Logger.h>
#include <cstdio>

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
    // @Range: 100 8000
    // @Units: PWM
    // @User: Standard
    AP_GROUPINFO("BEGIN", 2, VTX_CLI_Command, pwm_begin, 0),

    // @Param: END
    // @DisplayName: PWM End
    // @Description: Maximum PWM value to activate
    // @Range: 100 8000
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


/*
 * CLI команда формата Betaflight:
 * vtx_cli <index> <aux_channel> <band> <channel> <power> <pwm_low> <pwm_high>
 * 
 * Примеры:
 * vtx_cli 0 2 0 0 1 900 1100    - слот 0, AUX2, без band/chan, power=1, PWM 900-1100
 * vtx_cli 3 7 1 1 0 900 1100    - слот 3, AUX7, band A, channel 1, без power
 * vtx_cli                        - показать все настройки
 * vtx_cli help                   - показать справку
 */

void AP_VideoTX_CLI::handle_cli_command(AP_HAL::UARTDriver* port, char* cmd)
{
    AP_VideoTX_CLI* vtx_cli = get_singleton();
    if (vtx_cli == nullptr) {
        port->printf("VTX CLI not initialized\r\n");
        return;
    }
    vtx_cli->process_command(port, cmd);
}

void AP_VideoTX_CLI::process_command(AP_HAL::UARTDriver* port, char* cmd)
{
    // Пропустить пробелы
    while (*cmd == ' ') cmd++;
    
    // vtx_cli без аргументов - показать все
    if (*cmd == '\0' || *cmd == '\r' || *cmd == '\n') {
        print_all(port);
        return;
    }

    // vtx_cli help
    if (strncmp(cmd, "help", 4) == 0) {
        print_help(port);
        return;
    }

    // vtx_cli clear <index> - очистить слот
    if (strncmp(cmd, "clear", 5) == 0) {
        int index;
        if (sscanf(cmd + 5, "%d", &index) == 1) {
            if (index >= 0 && index < VTX_CLI_COMMANDS_MAX) {
                VTX_CLI_Command& slot = commands[index];
                slot.rc_channel.set_and_save(-1);
                slot.band.set_and_save(0);
                slot.channel.set_and_save(0);
                slot.power.set_and_save(0);
                slot.pwm_begin.set_and_save(0);
                slot.pwm_end.set_and_save(0);
                port->printf("vtx_cli %d cleared\r\n", index);
            } else {
                port->printf("Error: index must be 0-%d\r\n", VTX_CLI_COMMANDS_MAX - 1);
            }
        } else {
            port->printf("Usage: vtx_cli clear <index>\r\n");
        }
        return;
    }

    // Парсим: index aux band channel power pwm_low pwm_high
    int index, aux, band, channel, power, pwm_low, pwm_high;
    int parsed = sscanf(cmd, "%d %d %d %d %d %d %d", 
                        &index, &aux, &band, &channel, &power, &pwm_low, &pwm_high);

    if (parsed != 7) {
        port->printf("Error: need 7 args, got %d\r\n", parsed);
        print_help(port);
        return;
    }

    // Проверка диапазонов
    if (index < 0 || index >= VTX_CLI_COMMANDS_MAX) {
        port->printf("Error: index must be 0-%d\r\n", VTX_CLI_COMMANDS_MAX - 1);
        return;
    }

    if (aux < -1 || aux > 15) {
        port->printf("Error: aux must be -1 to 15\r\n");
        return;
    }

    if (band < 0 || band > 5) {
        port->printf("Error: band must be 0-5\r\n");
        return;
    }

    if (channel < 0 || channel > 8) {
        port->printf("Error: channel must be 0-8\r\n");
        return;
    }

    if (power < 0 || power > 5) {
        port->printf("Error: power must be 0-5\r\n");
        return;
    }

    if (pwm_low < 100 || pwm_low > 8000 || pwm_high < 100 || pwm_high > 8000) {
        port->printf("Error: pwm must be 800-2200\r\n");
        return;
    }

    if (pwm_low >= pwm_high) {
        port->printf("Error: pwm_low must be < pwm_high\r\n");
        return;
    }

    VTX_CLI_Command& slot = commands[index];
    
    slot.rc_channel.set_and_save(aux);
    slot.band.set_and_save(band);
    slot.channel.set_and_save(channel);
    slot.power.set_and_save(power);
    slot.pwm_begin.set_and_save(pwm_low);
    slot.pwm_end.set_and_save(pwm_high);

    port->printf("vtx_cli %d: RC%d band=%d chan=%d pwr=%d pwm=%d-%d saved\r\n",
                 index, aux, band, channel, power, pwm_low, pwm_high);
}

void AP_VideoTX_CLI::print_all(AP_HAL::UARTDriver* port)
{
    port->printf("# VTX_CLI settings\r\n");
    port->printf("# vtx_cli <idx> <rc> <band> <chan> <pwr> <pwm_lo> <pwm_hi>\r\n");
    
    for (uint8_t i = 0; i < VTX_CLI_COMMANDS_MAX; i++) {
        const VTX_CLI_Command& cmd = commands[i];
        
        port->printf("vtx_cli %d %d %d %d %d %d %d",
                     i,
                     (int)cmd.rc_channel.get(),
                     (int)cmd.band.get(),
                     (int)cmd.channel.get(),
                     (int)cmd.power.get(),
                     (int)cmd.pwm_begin.get(),
                     (int)cmd.pwm_end.get());
        
        if (cmd.is_configured()) {
            // Показать частоту если настроено
            static const uint16_t vtx_freq[5][8] = {
                {5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725},
                {5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866},
                {5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945},
                {5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880},
                {5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917},
            };
            
            port->printf("  #");
            if (cmd.band > 0 && cmd.channel > 0) {
                uint16_t freq = vtx_freq[cmd.band - 1][cmd.channel - 1];
                port->printf(" %dMHz", freq);
            }
            if (cmd.power > 0) {
                static const uint16_t pwr_mw[] = {25, 100, 200, 400, 600};
                port->printf(" %dmW", pwr_mw[cmd.power - 1]);
            }
        }
        port->printf("\r\n");
    }
}

void AP_VideoTX_CLI::print_help(AP_HAL::UARTDriver* port)
{
    port->printf(
        "VTX_CLI commands:\r\n"
        "  vtx_cli                 - show all\r\n"
        "  vtx_cli help            - this help\r\n"
        "  vtx_cli clear <idx>     - clear slot\r\n"
        "  vtx_cli <idx> <rc> <band> <ch> <pwr> <lo> <hi>\r\n"
        "\r\n"
        "  idx:  0-%d (slot number)\r\n"
        "  rc:   -1=off, 0-15 (RC channel)\r\n"
        "  band: 0=off, 1=A, 2=B, 3=E, 4=F, 5=R\r\n"
        "  ch:   0=off, 1-8\r\n"
        "  pwr:  0=off, 1=25, 2=100, 3=200, 4=400, 5=600mW\r\n"
        "  lo:   800-2200 (PWM low)\r\n"
        "  hi:   800-2200 (PWM high)\r\n"
        "\r\n"
        "Examples:\r\n"
        "  vtx_cli 0 6 0 0 1 900 1100   # RC6, 25mW\r\n"
        "  vtx_cli 1 6 0 0 4 1400 1600  # RC6, 400mW\r\n"
        "  vtx_cli 2 6 5 1 3 1900 2100  # RC6, R1, 200mW\r\n"
        "  vtx_cli clear 0              # clear slot 0\r\n",
        VTX_CLI_COMMANDS_MAX - 1
    );
}

namespace AP {
    AP_VideoTX_CLI* videotx_cli() {
        return AP_VideoTX_CLI::get_singleton();
    }
}

#endif