#pragma once

#include <stddef.h>

#define INA4235_CALIBRATE_CONST             0.00512

#define INA4235_CONFIG1_REG                 (0x20)
#define INA4235_CONFIG2_REG                 (0x21)

#define INA4235_CALIBRATION_CH1_REG         (0x05)
#define INA4235_CALIBRATION_CH2_REG         (0x0D)
#define INA4235_CALIBRATION_CH3_REG         (0x15)
#define INA4235_CALIBRATION_CH4_REG         (0x1D)
#define INA4235_CALIBRATION_CHn_REG(n)      ((n-1)*0x08 + 0x05)

#define INA4235_SHUNT_VOLTAGE_CH1_REG       (0x00)
#define INA4235_SHUNT_VOLTAGE_CH2_REG       (0x08)
#define INA4235_SHUNT_VOLTAGE_CH3_REG       (0x10)
#define INA4235_SHUNT_VOLTAGE_CH4_REG       (0x18)
#define INA4235_SHUNT_VOLTAGE_CHn_REG(n)    ((n-1)*0x08 + 0x00)

#define INA4235_BUS_VOLTAGE_CH1_REG         (0x01)
#define INA4235_BUS_VOLTAGE_CH2_REG         (0x09)
#define INA4235_BUS_VOLTAGE_CH3_REG         (0x11)
#define INA4235_BUS_VOLTAGE_CH4_REG         (0x19)
#define INA4235_BUS_VOLTAGE_CHn_REG(n)      ((n-1)*0x08 + 0x01)

#define INA4235_CURRENT_CH1_REG             (0x02)
#define INA4235_CURRENT_CH2_REG             (0x0A)
#define INA4235_CURRENT_CH3_REG             (0x12)
#define INA4235_CURRENT_CH4_REG             (0x1A)
#define INA4235_CURRENT_CHn_REG(n)          ((n-1)*0x08 + 0x02)

#define INA4235_POWER_CH1_REG               (0x03)
#define INA4235_POWER_CH2_REG               (0x0B)
#define INA4235_POWER_CH3_REG               (0x13)
#define INA4235_POWER_CH4_REG               (0x1B)
#define INA4235_POWER_CHn_REG(n)            ((n-1)*0x08 + 0x03)

#define INA4235_FLAGS_REG                   (0x22)
#define INA4235_MANUFACTURER_ID_REG         (0x7E)
#define INA4235_DEVICE_ID_REG               (0x7F)