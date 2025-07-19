/*

AX-12:
https://emanual.robotis.com/docs/en/dxl/ax/ax-12a/

PROTOCOL v2.0:
https://emanual.robotis.com/docs/en/dxl/protocol2/


###############################################################################
###############################################################################

Control Table of EEPROM Area :
-------------------------------------------------------------------------------

0 	2 	Model Number 	        Model Number 	                    R 	12
2 	1 	Firmware Version 	    Firmware Version 	                R 	-
3 	1 	ID 	                    DYNAMIXEL ID 	                    RW 	1
4 	1 	Baud Rate 	            Communication Speed                 RW 	1
5 	1 	Return Delay Time 	    Response Delay Time 	            RW 	250
6 	2 	CW Angle Limit 	        Clockwise Angle Limit 	            RW 	0
8 	2 	CCW Angle Limit 	    Counter-Clockwise Angle Limit 	    RW 	1023
11 	1 	Temperature Limit 	    Maximum Internal Temperature Limit 	RW 	70
12 	1 	Min Voltage Limit 	    Minimum Input Voltage Limit 	    RW 	60
13 	1 	Max Voltage Limit       Maximum Input Voltage Limit 	    RW 	140
14 	2 	Max Torque 	            Maximun Torque 	                    RW 	1023
16 	1 	Status Return Level     Select Types of Status Return 	    RW 	2
17 	1 	Alarm LED 	            LED for Alarm 	                    RW 	36
18 	1 	Shutdown 	            Shutdown Error Information 	        RW 	36

Control Table of RAM Area :
-------------------------------------------------------------------------------

24 	1 	Torque Enable 	        Motor Torque On/Off 	        RW 	0
25 	1 	LED 	                Status LED On/Off 	            RW 	0
26 	1 	CW Compliance Margin 	CW Compliance Margin 	        RW 	1
27 	1 	CCW Compliance Margin 	CCW Compliance Margin 	        RW 	1
28 	1 	CW Compliance Slope 	CW Compliance Slope 	        RW 	32
29 	1 	CCW Compliance Slope    CCW Compliance Slope 	        RW 	32
30 	2 	Goal Position 	        Target Position 	            RW 	-
32 	2 	Moving Speed 	        Moving Speed 	                RW 	-
34 	2 	Torque Limit 	        Torque Limit 	                RW 	Max Torque
36 	2 	Present Position 	    Present Position 	            R 	-
38 	2 	Present Speed 	        Present Speed               	R 	-
40 	2 	Present Load 	        Present Load 	                R 	-
42 	1 	Present Voltage 	    Present Voltage 	            R 	-
43 	1 	Present Temperature     Present Temperature 	        R 	-
44 	1 	Registered 	            If Instruction is registered 	R 	0
46 	1 	Moving 	                Movement Status 	            R 	0
47 	1 	Lock 	                Locking EEPROM              	RW 	0
48 	2 	Punch 	                Minimum Current Threshold   	RW 	32

###############################################################################
###############################################################################

*/

#ifndef __DYNAMIXEL_V2_H
#define __DYNAMIXEL_V2_H

#include "main.h"

/////////////////// MACROS ///////////////////

// Only necessary when using protocol v2.0
#define GET_INT16_LSB(bytes) (uint8_t)(((uint16_t)(bytes) >> 0) & 0xFF)
#define GET_INT16_MSB(bytes) (uint8_t)(((uint16_t)(bytes) >> 8) & 0xFF)

/////////////////// INSTRUCTIONS ///////////////////

#define PING                  0x01
#define READ                  0x02
#define WRITE                 0x03
#define REG_WRITE             0x04
#define ACTION                0x05
#define FACTORY_RESET         0x06
#define REBOOT                0x08
#define CLEAR                 0x10  // Exists only in protocol v2.0
#define CONTROL_TABLE_BACKUP  0x20  // Exists only in protocol v2.0
#define SYNC_READ             0x82  // Exists only in protocol v2.0
#define SYNC_WRITE            0x83
#define FAST_SYNC_READ        0x8A  // Exists only in protocol v2.0
#define BULK_READ             0x92
#define BULK_WRIE             0x93  // Exists only in protocol v2.0
#define FAST_BULK_READ        0x9A  // Exists only in protocol v2.0

/////////////////// ERRORS ///////////////////

// Protocole v1.0
#define INPUT_VOLTAGE_ERROR   0x01
#define ANGLE_LIMIT_ERROR     0x02
#define OVERHEATING_ERROR     0x04
#define RANGE_ERROR           0x08
#define CHECKSUL_ERROR        0x10
#define OVERLOAD_ERROR        0x20
#define INSTRUCTION_ERROR_V1  0x40

// Protocole v2.0
#define RESULT_FAIL           0x01
#define INSTRUCTION_ERROR_V2  0x02
#define CRC_ERROR             0x03
#define DATA_RANGE_ERROR      0x04
#define DATA_LENGHT_ERROR     0x05
#define DATA_LIMIT_ERROR      0x06
#define ACCESS_ERROR          0x07

#define FATAL_ERROR_BIT_MASK  0x80
#define ERROR_BIT_MASK        0x7F

/////////////////// CONTROL TABLE ///////////////////

// Specific to AX-12

// EEPROM AREA
#define ID_ADDR                  3
#define BAUD_RATE_ADDR           4
#define RETURN_DELAY_TIME_ADDR   5
#define CW_ANGLE_LIMIT_ADDR      6
#define CCW_ANGLE_LIMIT_ADDR     8
#define TEMP_LIMIT_ADDR         11
#define MIN_VOLTAGE_LIMIT_ADDR  12
#define MAX_VOLTAGE_LIMIT_ADDR  13
#define MAX_TORQUE_ADDR         14
#define STATUS_RET_LEVEL_ADDR   16
#define ALARM_LED_ADDR          17
#define SHUTDOWN_ADDR           18

// RAM AREA
#define TORQUE_ENABLE_ADDR      24
#define LED_ADDR                25
#define CW_COMP_MARGIN_ADDR     26
#define CCW_COMP_MARGIN_ADDR    27
#define CW_COMP_SLOPE_ADDR      28
#define CCW_COMP_SLOPE_ADDR     29
#define GOAL_POSITION_ADDR      30
#define MOVING_SPEED_ADDR       32
#define TORQUE_LIMIT_ADDR       34
#define PRESENT_POSITION_ADDR   36
#define PRESENT_SPEED_ADDR      38
#define PRESENT_LOAD_ADDR       40
#define PRESENT_VOLTAGE_ADDR    42
#define PRESENT_TEMP_ADDR       43
#define REGISTERED_ADDR         44
#define MOVING_ADDR             46
#define LOCK_ADDR               47
#define PUNCH_ADDR              48

/////////////////// CONSTANTS ///////////////////

// Comm
#define DYNAMIXEL_ID          0x01
#define BROADCAST_ID          0xFE
#define DEFAULT_TIMEOUT        500
#define MAX_BAUD_RATE      1000000

// Data
#define MAX_TORQUE            1023
#define MAX_MOVING_SPEED      1023
#define CW_ANGLE_LIMIT           0
#define CCW_ANGLE_LIMIT       1023
#define CW_DIRECTION             0
#define CCW_DIRECTION            1
#define LOCK                     1
#define JOINT_MODE               0
#define WHEEL_MODE               1

/////////////////// COMMON FUNCTIONS ///////////////////

// UART Half-Duplex Send and Receive functions
void uart_send(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length);
void uart_recv(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length);

// Send dynamixel packet
void send_packet_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t inst, uint8_t *params, uint8_t params_len);
void send_packet_v2(UART_HandleTypeDef *huart, uint8_t id, uint8_t inst, uint8_t *params, uint16_t params_len);

// Write into dynamixel registers
void dynamixel_write_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t address, uint8_t *data, uint8_t data_len);
void dynamixel_write_v2(UART_HandleTypeDef *huart, uint8_t id, uint16_t address, uint8_t *data, uint16_t data_len);

// Read dynamixel registers
uint8_t dynamixel_read_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t address, uint8_t data_len, uint8_t *r_data, uint8_t *r_data_len);
uint8_t dynamixel_read_v2(UART_HandleTypeDef *huart, uint8_t id, uint16_t address, uint16_t data_len, uint8_t *r_data, uint16_t *r_data_len);

// Parse dynamixel packet
uint8_t parse_status_packet_v1(uint8_t *packet, uint8_t packet_len, uint8_t *id, uint8_t *params, uint8_t *params_len, uint8_t *error, uint8_t *cks_check);
uint8_t parse_status_packet_v2(uint8_t *packet, uint32_t packet_len, uint8_t *id, uint8_t *params, uint16_t *params_len, uint8_t *error, uint8_t *crc_check);

// Calculate CRC
uint16_t update_crc(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size);

// Sync Write into dynamixel registers
void dynamixel_sync_write_v1(UART_HandleTypeDef *huart, uint8_t *ids, uint8_t address, uint8_t *data, uint8_t data_len, uint8_t nb_ids);
void dynamixel_sync_write_v2(UART_HandleTypeDef *huart, uint8_t *ids, uint16_t address, uint8_t *data, uint16_t data_len, uint8_t nb_ids);

/////////////////// SPECIFIC INSTRUCTION FUNCTIONS ///////////////////

// Set punch
void dynamixel_set_punch(UART_HandleTypeDef *huart, uint8_t id, uint16_t punch);

// Disable endless turn
void dynamixel_set_joint_mode(UART_HandleTypeDef *huart, uint8_t id);

// Set endless turn
void dynamixel_set_endless_turn(UART_HandleTypeDef *huart, uint8_t id, uint16_t velocity, uint8_t rot_dir);

// Set operation mode
void dynamixel_set_mode(UART_HandleTypeDef *huart, uint8_t id, uint16_t cw_angle_lim, uint16_t ccw_angle_lim);

// Set dynamixel ID
void dynamixel_set_id(UART_HandleTypeDef *huart, uint8_t id, uint8_t new_id);

// Set return delay time
void dynamixel_set_delay_time(UART_HandleTypeDef *huart, uint8_t id, uint8_t delay_time);

// Set Baudrate
void dynamixel_set_baudrate(UART_HandleTypeDef *huart, uint8_t id, uint8_t baudrate);

// Set LED
void dynamixel_set_led(UART_HandleTypeDef *huart, uint8_t id, uint8_t enable);

// Ping
void dynamixel_ping(UART_HandleTypeDef *huart, uint8_t id);

// Action
void dynamixel_action(UART_HandleTypeDef *huart);

// Factory reset
void dynamixel_reset(UART_HandleTypeDef *huart, uint8_t id);

// Enable torque
void dynamixel_set_torque_enable(UART_HandleTypeDef *huart, uint8_t id, uint8_t enable);

// Set goal position
void dynamixel_set_goal_position(UART_HandleTypeDef *huart, uint8_t id, uint16_t position);

// Set moving speed
void dynamixel_set_moving_speed(UART_HandleTypeDef *huart, uint8_t id, uint16_t velocity);

// Set moving speed and goal position at the same time
void dynamixel_set_position_and_velocity(UART_HandleTypeDef *huart, uint8_t id, uint16_t position, uint16_t velocity);

// Set maximum torque
void dynamixel_set_max_torque(UART_HandleTypeDef *huart, uint8_t id, uint16_t max_torque);

// Set compliance margin
void dynamixel_set_compliance_margin(UART_HandleTypeDef *huart, uint8_t id, uint8_t cw_margin, uint8_t ccw_margin);

// Set compliance slope
void dynamixel_set_compliance_slope(UART_HandleTypeDef *huart, uint8_t id, uint8_t cw_slope, uint8_t ccw_slope);

// Read
uint16_t dynamixel_read_present_position(UART_HandleTypeDef *huart, uint8_t id);
uint16_t dynamixel_read_present_moving_speed(UART_HandleTypeDef *huart, uint8_t id);
float dynamixel_read_present_load(UART_HandleTypeDef *huart, uint8_t id);
float dynamixel_read_present_voltage(UART_HandleTypeDef *huart, uint8_t id);
uint8_t dynamixel_read_present_temperature(UART_HandleTypeDef *huart, uint8_t id);
uint8_t dynamixel_read_registered(UART_HandleTypeDef *huart, uint8_t id);
uint8_t dynamixel_read_baudrate(UART_HandleTypeDef *huart, uint8_t id);
uint8_t dynamixel_read_moving(UART_HandleTypeDef *huart, uint8_t id);
uint8_t dynamixel_read_id(UART_HandleTypeDef *huart);

/////////////////// UTILITY FUNCTIONS ///////////////////

uint8_t baudrate_to_value(uint32_t baudrate);
uint32_t value_to_baudrate(uint8_t value);
uint16_t pos_to_value(float pos_deg);
uint16_t vel_to_value(float speed_deg_s);
uint16_t vel_to_value_wheel_mode(float speed_pct);
float value_to_load(uint16_t value);


#endif /* __DYNAMIXEL_V2_H */
