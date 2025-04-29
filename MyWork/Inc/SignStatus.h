/*
 * SignStatus.h
 *
 *  Created on: Aug 18, 2023
 *      Author: lq
 */

#ifndef INC_SIGNSTATUS_H_
#define INC_SIGNSTATUS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

    extern uint8_t signID;

#define CONSPICUITY_ALL_OFF 0
#define CONSPICUITY_ALL_FLASH 4
#define CONSPICUITY_ALL_ON 5

// signStatus.litsnsr_fault
#define LITSNSR1 (1 << 0)
#define LITSNSR2 (1 << 1)

// self_test
#define SELFTTEST_MASK (1 << 0)

// Fault status in single_led
#define FAULT_ST_FAN (1 << 0)
#define FAULT_ST_LITSNSR (1 << 1)
#define FAULT_ST_LANTERN (1 << 2)
#define FAULT_ST_CED (1 << 3)
#define FAULT_ST_CHAIN (1 << 4)
#define FAULT_ST_VOLTAGE (1 << 5)
#define FAULT_ST_TEMPERATURE (1 << 6)
#define FAULT_ST_HUMIDITY (1 << 7)

    typedef struct signStatus_t
    {
        uint8_t chain_fault;
        uint8_t over_temp;
        uint8_t self_test;
        uint8_t single_led; // MSProtocol V3.2, used for FAULT Status
        uint8_t lantn_fan_fault;
        uint8_t litsnsr_fault;
        uint8_t current_id;
        uint8_t next_id;
        uint16_t current_crc;
        uint16_t next_crc;
    } signStatus_t;
    extern signStatus_t signStatus;

    uint8_t GetFaultSt();
    void SetFaultSt(uint8_t);
    void ClrFaultSt(uint8_t);

    typedef struct signExtStatus_t
    {
        uint16_t hours;   // from boot up
        int16_t t_board;  // 0.1'C, 0 means error
        uint16_t lux[2];  // light sensor lux, 0 means error
        uint16_t vlux;    // light sensor lux, 0 means error
        uint8_t gain[4];
        uint8_t bright[4];
        uint8_t fault_led;     // number of faulty LEDs per tile
        uint16_t fl_cur; // flasher current
    } signExtStatus_t;
    extern signExtStatus_t signExtStatus;

// frame flag
#define IS_FRM_DEF (1 << 0)
    typedef struct frame_t
    {
        uint8_t conspicuity; // conspicuity byte from set frame command
        uint16_t crc;        // app crc from set frame command
    } frame_t;

#define FRAMES_SIZE 3
    const extern frame_t frames[FRAMES_SIZE]; // [0]: OFF, [1]: on, [2]: flashing

#ifdef __cplusplus
}
#endif

#endif /* INC_SIGNSTATUS_H_ */
