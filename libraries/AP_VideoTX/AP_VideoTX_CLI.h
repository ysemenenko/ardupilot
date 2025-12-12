#pragma once

#include <AP_Param/AP_Param.h>
#include <GCS_MAVLink/GCS.h>
#include <RC_Channel/RC_Channel.h>

#ifndef HAL_VIDEOTX_CLI_ENABLED
#define HAL_VIDEOTX_CLI_ENABLED 1
#endif

#if defined(HAL_VIDEOTX_CLI_ENABLED) && HAL_VIDEOTX_CLI_ENABLED

#define VTX_CLI_COMMANDS_MAX 8

class VTX_CLI_Command {
public:
    AP_Int8  rc_channel;
    AP_Int16 pwm_begin;
    AP_Int16 pwm_end;
    AP_Int8  band;
    AP_Int8  channel;
    AP_Int8  power;

    static const struct AP_Param::GroupInfo var_info[];

    bool in_range(uint16_t pwm) const {
        return (pwm >= pwm_begin) && (pwm <= pwm_end);
    }

    bool is_active() const {
        if (rc_channel < 0) {
            return false;
        }
        
        RC_Channels* rc = RC_Channels::get_singleton();
        if (rc == nullptr) {
            return false;
        }
        
        RC_Channel* ch = rc->channel((uint8_t)rc_channel.get());
        if (ch == nullptr) {
            return false;
        }
        
        return in_range(ch->get_radio_in());
    }

    bool is_configured() const {
        return rc_channel >= 0 && pwm_begin > 0 && pwm_end > 0;
    }
};

class AP_VideoTX_CLI {
public:
    AP_VideoTX_CLI();
    
    CLASS_NO_COPY(AP_VideoTX_CLI);

    static AP_VideoTX_CLI* get_singleton() { return _singleton; }

    void init();
    void update();

    const VTX_CLI_Command* get_command(uint8_t index) const;
    const VTX_CLI_Command* get_active_command() const;

    static const struct AP_Param::GroupInfo var_info[];

    VTX_CLI_Command commands[VTX_CLI_COMMANDS_MAX];

    // CLI command handler
    static void handle_cli_command(AP_HAL::UARTDriver* port, char* cmd);
    void process_command(AP_HAL::UARTDriver* port, char* cmd);
    void print_all(AP_HAL::UARTDriver* port);
    void print_help(AP_HAL::UARTDriver* port);
    
private:
    static AP_VideoTX_CLI* _singleton;

    bool _initialized;
    uint32_t _last_update_ms;
    int8_t _last_active_idx;

    static constexpr uint16_t UPDATE_INTERVAL_MS = 100;

    void apply_command(const VTX_CLI_Command& cmd);
    void send_status_msg(const VTX_CLI_Command& cmd, uint8_t idx);
};

namespace AP {
    AP_VideoTX_CLI* videotx_cli();
}

#endif