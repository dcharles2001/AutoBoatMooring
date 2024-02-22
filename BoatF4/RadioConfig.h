#ifndef RADIOCONFIG_H
#define RADIOCONFIG_H

#include "Patch.h"

// USER DEFINED PARAMETERS
// Define your own parameters here

// INPUT DATA
/*
// Crys_freq(Hz): 30000000    Crys_tol(ppm): 30    IF_mode: 2    High_perf_Ch_Fil: 1    OSRtune: 0    Ch_Fil_Bw_AFC: 0    ANT_DIV: 0    PM_pattern: 0    
// MOD_type: 2    Rsymb(sps): 2400    Fdev(Hz): 30000    RXBW(Hz): 114000    Manchester: 0    AFC_en: 1    Rsymb_error: 0.0    Chip-Version: 2    
// RF Freq.(MHz): 433    API_TC: 28    fhst: 250000    inputBW: 0    BERT: 0    RAW_dout: 0    D_source: 0    Hi_pfm_div: 0    
// 
// # WB filter 1 (BW = 114.46 kHz);  NB-filter 1 (BW = 114.46 kHz)  // 
// Modulation index: 25
*/


// CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ                     30000000L
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER                    0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH               0x07
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP        0x03
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET       0xF000



// CONFIGURATION COMMANDS

/*
// Command:                  RF_POWER_UP
// Description:              Command to power-up the device and select the operational mode and functionality.
*/
#define RF_POWER_UP 0x02, 0x01, 0x00, 0x01, 0xC9, 0xC3, 0x80

/*
// Set properties:           RF_INT_CTL_ENABLE_1
// Number of properties:     1
// Group ID:                 0x01
// Start ID:                 0x00
// Default values:           0x04, 
// Descriptions:
//   INT_CTL_ENABLE - This property provides for global enabling of the three interrupt groups (Chip, Modem and Packet Handler) in order to generate HW interrupts at the NIRQ pin.
*/
#define RF_INT_CTL_ENABLE_1 0x11, 0x01, 0x01, 0x00, 0x00

/*
// Set properties:           RF_FRR_CTL_A_MODE_4
// Number of properties:     4
// Group ID:                 0x02
// Start ID:                 0x00
// Default values:           0x01, 0x02, 0x09, 0x00, 
// Descriptions:
//   FRR_CTL_A_MODE - Fast Response Register A Configuration.
//   FRR_CTL_B_MODE - Fast Response Register B Configuration.
//   FRR_CTL_C_MODE - Fast Response Register C Configuration.
//   FRR_CTL_D_MODE - Fast Response Register D Configuration.
*/
#define RF_FRR_CTL_A_MODE_4 0x11, 0x02, 0x04, 0x00, 0x08, 0x06, 0x04, 0x0A

/*
// Set properties:           RF_EZCONFIG_XO_TUNE_1
// Number of properties:     1
// Group ID:                 0x24
// Start ID:                 0x03
// Default values:           0x40, 
// Descriptions:
//   EZCONFIG_XO_TUNE - Configure the internal capacitor frequency tuning bank for the crystal oscillator.
*/
#define RF_EZCONFIG_XO_TUNE_1 0x11, 0x24, 0x01, 0x03, 0x52

/*
// Command:                  RF_WRITE_TX_FIFO
// Description:              Writes data byte(s) to the TX FIFO.
*/
#define RF_WRITE_TX_FIFO 0x66, 0xA8, 0xF7, 0xBD, 0x20, 0xC8, 0x43, 0x3C, 0x01, 0x76, 0x33, 0xBB, 0x24, 0x9B, 0x38, 0x11, 0x51, 0x77, 0xD8, 0xCA, \
0x8F, 0xA0, 0x23, 0x64, 0xFD, 0x92, 0xE6, 0x14, 0x96, 0xE6, 0x7D, 0x7F, 0x20, 0x32, 0x62, 0x3B, 0x7F, 0xC9, 0x9F, 0x15, \
0x5F, 0x47, 0x4C, 0x05, 0x1A, 0x53, 0x5E, 0x6F, 0x04, 0xB3, 0xB0, 0xA0, 0xF5, 0x9B, 0xD7, 0xCB, 0xB6, 0x9E, 0x00, 0xE3, \
0x02, 0xBE, 0xC3, 0x38, 0x45, 0x6F, 0xF3, 0x65, 0x28, 0xA0, 0x35, 0x7A, 0x0E, 0x94, 0x78, 0x4D, 0x9B, 0xF6, 0x93, 0x7E, \
0x45, 0x97, 0xE2, 0x1A, 0x9D, 0xAC, 0xAD, 0xA7, 0x9D, 0x68, 0x18, 0xF2, 0x9C, 0x92, 0xEB, 0xD9, 0x49, 0xF3, 0x9B, 0xFC, \
0xEB, 0xD9, 0xEB, 0x50, 0xCD, 0xA7, 0x80, 0x87, 0xBA, 0x89, 0x33, 0xB0, 0x64, 0xE1

/*
// Command:                  RF_NOP
// Description:              No Operation command.
*/
#define RF_NOP 0x00

/*
// Command:                  RF_WRITE_TX_FIFO_1
// Description:              Writes data byte(s) to the TX FIFO.
*/
#define RF_WRITE_TX_FIFO_1 0x66, 0x5F, 0xC1, 0xE7, 0x84, 0x51, 0x8A, 0x36, 0xAE, 0xA0, 0x12, 0xF7, 0xD2, 0xD4, 0x21, 0xF5, 0x90, 0xE7, 0x10, 0xF6, \
0xA5, 0x08, 0xCD, 0xD8, 0xA5, 0x39, 0x0D, 0x3F, 0x7E, 0x9A, 0x65, 0x1F, 0x2D, 0xEE, 0xE8, 0x57, 0x4D, 0x10, 0xE0, 0x96, \
0x3F, 0x34, 0x13, 0xFB, 0x8B, 0x22, 0xD2, 0x85, 0x30, 0x89, 0x1D, 0x70, 0x93, 0x5F, 0x8F, 0x4A, 0x66, 0xEF, 0x17, 0xBF, \
0x08, 0xB9, 0x8E, 0x1E, 0x9D, 0x0B, 0x56, 0x62, 0x43, 0xCE, 0x05, 0x18, 0xB5, 0xFE, 0x9B, 0xF7, 0x99, 0x60, 0x38, 0x73, \
0x41, 0xA8, 0x5E, 0x94, 0x4E, 0xDB, 0x53, 0x9C, 0x5F, 0x8D, 0xCB, 0x26, 0xFD, 0x8E, 0x42, 0x06, 0x09, 0x20, 0x9E, 0x2E, \
0x54, 0x1E, 0x9B, 0xB4, 0x1C, 0xF8, 0x73, 0x99, 0x81, 0x5D, 0xE3, 0x2D

/*
// Command:                  RF_EZCONFIG_CHECK
// Description:              Validates the EZConfig array was written correctly.
*/
#define RF_EZCONFIG_CHECK 0x19, 0xAD, 0xEC

/*
// Command:                  RF_GPIO_PIN_CFG
// Description:              Configures the GPIO pins.
*/
#define RF_GPIO_PIN_CFG 0x13, 0x00, 0x00, 0x01, 0x00, 0x00, 0x0B, 0x00


// AUTOMATICALLY GENERATED CODE! 
// DO NOT EDIT/MODIFY BELOW THIS LINE!
// --------------------------------------------

#ifndef FIRMWARE_LOAD_COMPILE
#define RADIO_CONFIGURATION_DATA_ARRAY { \
        0x07, RF_POWER_UP, \
        0x05, RF_INT_CTL_ENABLE_1, \
        0x08, RF_FRR_CTL_A_MODE_4, \
        0x05, RF_EZCONFIG_XO_TUNE_1, \
        0x72, RF_WRITE_TX_FIFO, \
        0x01, RF_NOP, \
        0x70, RF_WRITE_TX_FIFO_1, \
        0x03, RF_EZCONFIG_CHECK, \
        0x08, RF_GPIO_PIN_CFG, \
        0x00 \
 }
#else
#define RADIO_CONFIGURATION_DATA_ARRAY { 0 }
#endif

// DEFAULT VALUES FOR CONFIGURATION PARAMETERS
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT                     30000000L
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT                    0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH_DEFAULT               0x10
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP_DEFAULT        0x01
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET_DEFAULT       0x1000

#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_INCLUDED                      0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH_SIZE                          0x00
#define RADIO_CONFIGURATION_DATA_RADIO_PATCH                               {  }

#ifndef RADIO_CONFIGURATION_DATA_ARRAY
#error "This property must be defined!"
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ
#define RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ          RADIO_CONFIGURATION_DATA_RADIO_XO_FREQ_DEFAULT 
#endif

#ifndef RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER
#define RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER         RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER_DEFAULT 
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH
#define RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH    RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH_DEFAULT 
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP
#define RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP   RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP_DEFAULT 
#endif

#ifndef RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET
#define RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET  RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET_DEFAULT 
#endif

#define RADIO_CONFIGURATION_DATA { \
                            Radio_Configuration_Data_Array,                            \
                            RADIO_CONFIGURATION_DATA_CHANNEL_NUMBER,                   \
                            RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH,              \
                            RADIO_CONFIGURATION_DATA_RADIO_STATE_AFTER_POWER_UP,       \
                            RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET       \
                            }

#endif //RADIOCONFIG_H
