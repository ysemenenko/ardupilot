// AP_VideoTX_CLI.h
#pragma once

#include <AP_Param/AP_Param.h>
#include <RC_Channel/RC_Channel.h>

#define VTX_CLI_NUM_COMMANDS 8

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

    static AP_VideoTX_CLI *get_singleton();
    static const struct AP_Param::GroupInfo var_info[];

    // Находит активную команду (первую по приоритету)
    const VTX_CLI_Command* get_active_command() const;

    VTX_CLI_Command commands[VTX_CLI_NUM_COMMANDS];

private:
    static AP_VideoTX_CLI *_singleton;
};

namespace AP {
    AP_VideoTX_CLI *vtx_cli();
};