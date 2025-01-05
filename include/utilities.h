#pragma once

// Note:
// When using ArduinoIDE, you must select a corresponding board type.
// If you don’t know which board type you have, please click on the link to view it.

// Products Link:https://www.lilygo.cc/products/t-sim-a7670e
// #define LILYGO_T_A7670


#if defined(LILYGO_T_A7670)

#define MODEM_BAUDRATE                      (115200)
#define MODEM_DTR_PIN                       (25)
#define MODEM_TX_PIN                        (26)
#define MODEM_RX_PIN                        (27)
// The modem boot pin needs to follow the startup sequence.
#define BOARD_PWRKEY_PIN                    (4)
#define BOARD_ADC_PIN                       (35)
// The modem power switch must be set to HIGH for the modem to supply power.
#define BOARD_POWERON_PIN                   (12)
#define MODEM_RING_PIN                      (33)
#define MODEM_RESET_PIN                     (5)
#define BOARD_MISO_PIN                      (2)
#define BOARD_MOSI_PIN                      (15)
#define BOARD_SCK_PIN                       (14)
#define BOARD_SD_CS_PIN                     (13)
#define BOARD_BAT_ADC_PIN                   (35)
#define MODEM_RESET_LEVEL                   HIGH
#define SerialAT                            Serial1

#define MODEM_GPS_ENABLE_GPIO               (-1)
#define MODEM_GPS_ENABLE_LEVEL              (-1)

#ifndef TINY_GSM_MODEM_A7670
#define TINY_GSM_MODEM_A7670
#endif

// It is only available in V1.4 version. In other versions, IO36 is not connected.
#define BOARD_SOLAR_ADC_PIN                 (36)

#else
#error "Use ArduinoIDE, please open the macro definition corresponding to the board above <utilities.h>"
#endif
