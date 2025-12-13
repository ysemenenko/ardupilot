#pragma once

#define DEFAULT_LOOP_RATE 400
#define YC_NUM_INSTANCES 4

#include <AP_Param/AP_Param.h>

class AP_YCustom
{
public:
    AP_YCustom();

    /* Do not allow copies */
    CLASS_NO_COPY(AP_YCustom);

    static AP_YCustom *get_singleton();
    static AP_YCustom *_singleton;

    static const struct AP_Param::GroupInfo var_info[];

    // Вложенный класс для параметров каждого экземпляра
    class Params {
    public:
        static const struct AP_Param::GroupInfo var_info[];
        
        AP_Int8 debug;
        AP_Int16 loop_rate_hz;
    };

    Params params[YC_NUM_INSTANCES];

private:
};

namespace AP {
    AP_YCustom &ycustom();
};