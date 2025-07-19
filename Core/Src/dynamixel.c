#include <dynamixel.h>


// PULL-UP RESISTOR: 1.8K
// BAUDRATE: 117 647 (WHEN BAD COMMUNICATION WITH 115 200)


void uart_send(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length)
{
	// UART_FLAG_TXE : Transmit data register empty flag
	// UART_FLAG_TC  : Transmission Complete flag

	HAL_HalfDuplex_EnableTransmitter(huart);
	HAL_UART_Transmit(huart, data, length, DEFAULT_TIMEOUT);
	HAL_HalfDuplex_EnableReceiver(huart);
}

void uart_recv(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length)
{
	// UART_FLAG_RXNE: Receive data register not empty flag
	// UART_FLAG_IDLE: IDLE line detection

	HAL_UART_Receive(huart, data, length, DEFAULT_TIMEOUT);
}

void send_packet_v2(UART_HandleTypeDef *huart, uint8_t id, uint8_t inst, uint8_t *params, uint16_t params_len)
{
	uint16_t packet_length = 10 + params_len;
	uint8_t packet[packet_length];

	packet[0] = 0xFF; // Header 1
	packet[1] = 0xFF; // Header 2
	packet[2] = 0xFD; // Header 3
	packet[3] = 0x00; // Reserved
	packet[4] = id;   // Packet ID

	// Length = Parameter length + 3
	packet[5] = GET_INT16_LSB(params_len + 3); // Length 1 (LSB)
	packet[6] = GET_INT16_MSB(params_len + 3); // Length 2 (MSB)

	packet[7] = (uint8_t)inst; // Instrucion

	// Parameter 1 to X
	for (uint16_t i = 0; i < params_len; i++)
		packet[8 + i] = params[i];

	// CRC
	uint16_t crc = update_crc(0, packet, packet_length - 2); // Calculating CRC
	packet[packet_length - 2] = GET_INT16_LSB(crc);          // CRC 1 (LSB)
	packet[packet_length - 1] = GET_INT16_MSB(crc);          // CRC 2 (MSB)

	uart_send(huart, packet, packet_length);

}

void send_packet_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t inst, uint8_t *params, uint8_t params_len)
{
	uint8_t packet_length = 6 + params_len;
	uint16_t checksum = id + params_len + 2 + inst;
	uint8_t packet[packet_length];

	packet[0] = 0xFF;            // Header 1
	packet[1] = 0xFF;            // Header 2
	packet[2] = id;              // Packet ID
	packet[3] = params_len + 2;  // Length = Parameter length + 2
	packet[4] = inst;            // Instruction

	// Parameters 1 to X
	for (uint16_t i = 0; i < params_len; i++)
	{
		packet[5 + i] = params[i];
		checksum += params[i];
	}

	// Checksum
	packet[packet_length - 1] = (uint8_t)(~checksum & 0xFF);

	uart_send(huart, packet, packet_length);
}

void dynamixel_write_v2(UART_HandleTypeDef *huart, uint8_t id, uint16_t address, uint8_t *data, uint16_t data_len)
{
	uint16_t params_length = data_len + 2;
	uint8_t params[params_length];

	// Parameter 1 to 2: Address
	params[0] = GET_INT16_LSB(address);
	params[1] = GET_INT16_MSB(address);

	// Parameter 3 to X: Data
	for (uint16_t i = 0; i < data_len; i++)
		params[2 + i] = data[i];

	send_packet_v2(huart, id, WRITE, params, params_length);
}

void dynamixel_write_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t address, uint8_t *data, uint8_t data_len)
{
	uint8_t params_length = data_len + 1;
	uint8_t params[params_length];

	// Parameter 1: Address
	params[0] = address;

	// Parameter 2 to X: Data
	for (uint16_t i = 0; i < data_len; i++)
		params[1 + i] = data[i];

	send_packet_v1(huart, id, WRITE, params, params_length);
}

uint8_t dynamixel_read_v2(UART_HandleTypeDef *huart, uint8_t id, uint16_t address, uint16_t data_len, uint8_t *r_data, uint16_t *r_data_len)
{
	uint8_t params[4];
	uint8_t status_packet[64];
	uint8_t status_packet_length = 64;

	// Parameter 1-2: Address
	params[0] = GET_INT16_LSB(address);
	params[1] = GET_INT16_MSB(address);

	// Parameter 2-3: Data length
	params[2] = GET_INT16_LSB(data_len);
	params[3] = GET_INT16_MSB(data_len);

	// Send and receive
	send_packet_v1(huart, id, READ, params, 4);
	uart_recv(huart, status_packet, status_packet_length);

	uint8_t id_r;
	uint8_t error;
	uint8_t crc_check;

	parse_status_packet_v2(status_packet, status_packet_length, &id_r, r_data, r_data_len, &error, &crc_check);

	return (id_r == id) && (error == 0x00) && (crc_check);
}

uint8_t dynamixel_read_v1(UART_HandleTypeDef *huart, uint8_t id, uint8_t address, uint8_t data_len, uint8_t *r_data, uint8_t *r_data_len)
{
	uint8_t params[4];
	uint8_t status_packet[64];
	uint8_t status_packet_length = 64;

	params[0] = address;   // Parameter 1: Address
	params[1] = data_len;  // Parameter 2: Data length

	// Send and Receive
	send_packet_v1(huart, id, READ, params, 2);
	uart_recv(huart, status_packet, status_packet_length);

	uint8_t id_r;
	uint8_t error;
	uint8_t cks_check;

	parse_status_packet_v1(status_packet, status_packet_length, &id_r, r_data, r_data_len, &error, &cks_check);

	return (id_r == id) && (error == 0x00) && (cks_check);
}

uint8_t parse_status_packet_v2(uint8_t *packet, uint32_t packet_len, uint8_t *id, uint8_t *params, uint16_t *params_len, uint8_t *error, uint8_t *crc_check)
{
	// Check instruction
	if (packet[7] != 0x55)
		return 0;

	*id = packet[4];
	*error = packet[8];

	uint16_t length = packet[5] + ((uint16_t)(packet[6] << 8) & 0xFF00);
	*params_len = length - 3 - 1;

	for (uint16_t i = 0; i < *params_len; i++)
	  params[i] = packet[9 + i];

	// CRC check
	uint16_t crc = update_crc(0, packet, packet_len - 2);          // Calculating CRC
	uint8_t crc_l = packet[packet_len - 2] == GET_INT16_LSB(crc);  // CRC 1 (LSB)
	uint8_t crc_h = packet[packet_len - 1] == GET_INT16_MSB(crc);  // CRC 2 (MSB)
	*crc_check = (crc_l && crc_h);

	return 1;
}

uint8_t parse_status_packet_v1(uint8_t *packet, uint8_t packet_len, uint8_t *id, uint8_t *params, uint8_t *params_len, uint8_t *error, uint8_t *cks_check)
{
	uint8_t length = packet[4];
	uint16_t checksum = packet[3] + packet[5] + length;

	*id = packet[3];
	*error = packet[5];

	*params_len = length - 2;

	for (uint16_t i = 0; i < *params_len; i++)
	{
		params[i] = packet[6 + i];
		checksum += packet[6 + i];
	}

	// Checksum check
	*cks_check = packet[packet_len - 1] == (uint8_t)(~checksum & 0xFF);

	return 1;
}

void dynamixel_sync_write_v1(UART_HandleTypeDef *huart, uint8_t *ids, uint8_t address, uint8_t *data, uint8_t data_len, uint8_t nb_ids)
{
	uint8_t params_len = (data_len + 1) * nb_ids + 2;
	uint8_t params[params_len];

	params[0] = address;
	params[1] = data_len;

	for (uint16_t i = 0; i < params_len - 2; i++)
	{
		if (i % (data_len + 1) == 0)
			params[2 + i] = ids[i / (data_len + 1)];
		else
			params[2 + i] = data[i - 1 - (i / (data_len + 1)];
	}

	send_packet_v1(huart, BROADCAST_ID, SYNC_WRITE, params, params_len);
}

void dynamixel_sync_write_v2(UART_HandleTypeDef *huart, uint8_t *ids, uint16_t address, uint8_t *data, uint16_t data_len, uint8_t nb_ids)
{
	uint16_t params_len = (data_len + 1) * nb_ids + 4;
	uint8_t params[params_len];

	params[0] = GET_INT16_LSB(address);
	params[1] = GET_INT16_MSB(address);

	params[2] = GET_INT16_LSB(data_len);
	params[3] = GET_INT16_MSB(data_len);

	for (uint16_t i = 0; i < params_len - 4; i++)
	{
		if (i % data_len == 0)
			params[4 + i] = ids[i / data_len];
		else
			params[4 + i] = data[i - 1 - (i / data_len)];
	}

	send_packet_v2(huart, BROADCAST_ID, SYNC_WRITE, params, params_len);
}


// ============================================================================================================== //
// SPECIFIC FUNCTIONS
// ============================================================================================================== //

void dynamixel_set_mode(UART_HandleTypeDef *huart, uint8_t id, uint16_t cw_angle_lim, uint16_t ccw_angle_lim)
{
	uint16_t starting_address = CW_ANGLE_LIMIT_ADDR;
	uint8_t data[4];

	data[0] = (uint8_t)((cw_angle_lim >> 0) & 0xFF);
	data[1] = (uint8_t)((cw_angle_lim >> 8) & 0xFF);

	data[2] = (uint8_t)((ccw_angle_lim >> 0) & 0xFF);
	data[3] = (uint8_t)((ccw_angle_lim >> 8) & 0xFF);

	// Write CW and CCW Angle Limits
	dynamixel_write_v1(huart, id, starting_address, data, 4);
}

void dynamixel_set_joint_mode(UART_HandleTypeDef *huart, uint8_t id)
{
	dynamixel_set_mode(huart, id, 0, 1023);
}

void dynamixel_set_endless_turn(UART_HandleTypeDef *huart, uint8_t id, uint16_t velocity, uint8_t rot_dir)
{
	// Wheel mode for endless turn
	dynamixel_set_mode(huart, id, 0, 0);

	// Calculate velocity according to direction
	if (rot_dir == CW_DIRECTION)
		velocity |= 0x400;
	else if (rot_dir == CCW_DIRECTION)
		velocity &= 0x7FF;
	else
		velocity |= 0x7FF;

	// Write moving speed
	dynamixel_set_moving_speed(huart, id, velocity);
}

void dynamixel_set_id(UART_HandleTypeDef *huart, uint8_t id, uint8_t new_id)
{
	uint16_t address = ID_ADDR;
	uint8_t data = new_id;

	dynamixel_write_v1(huart, id, address, &data, 1);
}

void dynamixel_set_baudrate(UART_HandleTypeDef *huart, uint8_t id, uint8_t baudrate)
{
	uint16_t address = BAUD_RATE_ADDR;
	uint8_t data = baudrate;

	// We want a baudrate of 115200 by default
	if (baudrate == 0xFF)
		data = 16;

	dynamixel_write_v1(huart, id, address, &data, 1);
}

void dynamixel_set_led(UART_HandleTypeDef *huart, uint8_t id, uint8_t enable)
{
	uint16_t address = LED_ADDR;
	uint8_t data = enable & 0x1;

	dynamixel_write_v1(huart, id, address, &data, 1);
}

void dynamixel_set_delay_time(UART_HandleTypeDef *huart, uint8_t id, uint8_t delay_time)
{
	uint16_t address = LED_ADDR;
	uint8_t data = delay_time >> 1;

	dynamixel_write_v1(huart, id, address, &data, 1);
}

void dynamixel_ping(UART_HandleTypeDef *huart, uint8_t id)
{
	send_packet_v1(huart, id, PING, NULL, 0);
}

void dynamixel_action(UART_HandleTypeDef *huart)
{
	send_packet_v1(huart, BROADCAST_ID, ACTION, NULL, 0);
}

void dynamixel_reset(UART_HandleTypeDef *huart, uint8_t id)
{
	// 0xFF: Reset all
	// 0x01: Reset all except ID
	// 0x02: reset all except ID and Baudrate

	uint8_t parameter = 0x02;
	send_packet_v1(huart, id, FACTORY_RESET, &parameter, 1);
}

void dynamixel_set_torque_enable(UART_HandleTypeDef *huart, uint8_t id, uint8_t enable)
{
	uint16_t address = TORQUE_ENABLE_ADDR;
	uint8_t data = enable & 0x1;

	dynamixel_write_v1(huart, id, address, &data, 1);
}

void dynamixel_set_goal_position(UART_HandleTypeDef *huart, uint8_t id, uint16_t position)
{
	uint16_t address = GOAL_POSITION_ADDR;
	uint8_t data[2];

	data[0] = (uint8_t)((position >> 0) & 0xFF);
	data[1] = (uint8_t)((position >> 8) & 0xFF);

	dynamixel_write_v1(huart, id, address, data, 2);
}

void dynamixel_set_moving_speed(UART_HandleTypeDef *huart, uint8_t id, uint16_t speed)
{
	uint16_t address = MOVING_SPEED_ADDR;
	uint8_t data[2];

	data[0] = (uint8_t)((speed >> 0) & 0xFF);
	data[1] = (uint8_t)((speed >> 8) & 0xFF);

	dynamixel_write_v1(huart, id, address, data, 2);
}

uint16_t dynamixel_read_present_position(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = PRESENT_POSITION_ADDR;
	uint8_t return_data[2];
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 2, return_data, &return_data_length);

	uint16_t position = ((return_data[0] << 0) & 0x00FF) +
						((return_data[1] << 8) & 0xFF00);

	return position;
}

uint16_t dynamixel_read_present_moving_speed(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = PRESENT_SPEED_ADDR;
	uint8_t return_data[2];
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 2, return_data, &return_data_length);

	uint16_t moving_speed = ((return_data[0] << 0) & 0x00FF) +
							((return_data[1] << 8) & 0xFF00);

	return moving_speed;
}

float dynamixel_read_present_load(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = PRESENT_LOAD_ADDR;
	uint8_t return_data[2];
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 2, return_data, &return_data_length);

	uint16_t load = ((return_data[0] << 0) & 0x00FF) +
					((return_data[1] << 8) & 0xFF00);

	return value_to_load(load);
}

float dynamixel_read_present_voltage(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = PRESENT_VOLTAGE_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 1, &return_data, &return_data_length);

	return return_data / 10.0;
}

uint8_t dynamixel_read_present_temperature(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = PRESENT_TEMP_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 1, &return_data, &return_data_length);

	return return_data;
}

uint8_t dynamixel_read_id(UART_HandleTypeDef *huart)
{
	uint16_t address = ID_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, BROADCAST_ID, address, 1, &return_data, &return_data_length);

	return return_data;
}

uint8_t dynamixel_read_registered(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = REGISTERED_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 1, &return_data, &return_data_length);

	return return_data & 0x01;
}

uint8_t dynamixel_read_baudrate(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = BAUD_RATE_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 1, &return_data, &return_data_length);

	return return_data;
}

uint8_t dynamixel_read_moving(UART_HandleTypeDef *huart, uint8_t id)
{
	uint16_t address = MOVING_ADDR;
	uint8_t return_data;
	uint8_t return_data_length;

	dynamixel_read_v1(huart, id, address, 1, &return_data, &return_data_length);

	return return_data & 0x01;
}

void dynamixel_set_position_and_velocity(UART_HandleTypeDef *huart, uint8_t id, uint16_t position, uint16_t velocity)
{
	uint16_t starting_address = GOAL_POSITION_ADDR;
	uint8_t data[4];

	// Goal position
	data[0] = (uint8_t)((position >> 0) & 0xFF);
	data[1] = (uint8_t)((position >> 8) & 0xFF);

	// Moving speed
	data[2] = (uint8_t)((velocity >> 0) & 0xFF);
	data[3] = (uint8_t)((velocity >> 8) & 0xFF);

	dynamixel_write_v1(huart, id, starting_address, data, 4);
}

void dynamixel_set_max_torque(UART_HandleTypeDef *huart, uint8_t id, uint16_t max_torque)
{
	uint16_t address = MAX_TORQUE_ADDR;
	uint8_t data[2];

	data[0] = (uint8_t)((max_torque >> 0) & 0xFF);
	data[1] = (uint8_t)((max_torque >> 8) & 0xFF);

	dynamixel_write_v1(huart, id, address, data, 2);
}

void dynamixel_set_compliance_margin(UART_HandleTypeDef *huart, uint8_t id, uint8_t cw_margin, uint8_t ccw_margin)
{
	uint16_t starting_address = CW_COMP_MARGIN_ADDR;
	uint8_t data[2] = {cw_margin, ccw_margin};

	dynamixel_write_v1(huart, id, starting_address, data, 2);
}

void dynamixel_set_compliance_slope(UART_HandleTypeDef *huart, uint8_t id, uint8_t cw_slope, uint8_t ccw_slope)
{
	uint16_t starting_address = CW_COMP_SLOPE_ADDR;
	uint8_t data[2] = {cw_slope, ccw_slope};

	dynamixel_write_v1(huart, id, starting_address, data, 2);
}

void dynamixel_set_punch(UART_HandleTypeDef *huart, uint8_t id, uint16_t punch)
{
	uint16_t starting_address = PUNCH_ADDR;
	uint8_t data[2];

	data[0] = (uint8_t)((punch >> 0) & 0xFF);
	data[1] = (uint8_t)((punch >> 8) & 0xFF);

	dynamixel_write_v1(huart, id, starting_address, data, 2);
}

// ============================================================================================================== //
// UTILITY FUNCTIONS
// ============================================================================================================== //

uint8_t baudrate_to_value(uint32_t baudrate)
{
	return (uint8_t)((2000000 / baudrate) - 1);
}

uint32_t value_to_baudrate(uint8_t value)
{
	return (uint32_t)(2000000 / (value + 1));
}

uint16_t pos_to_value(float pos_deg)
{
	return (uint16_t)(pos_deg * 3.41);
}

uint16_t vel_to_value(float speed_deg_s)
{
	return (uint16_t)(speed_deg_s * 1.50);
}

uint16_t vel_to_value_wheel_mode(float speed_pct)
{
	return (uint16_t)(speed_pct * 10.24);
}

float value_to_load(uint16_t value)
{
	float load = (value & 0x3FF) * 0.097;

	if (((value >> 10) & 1) == 0)
		load = -load;

	return load;
}

// Used only by protocol v2.0
// Source: https://emanual.robotis.com/docs/en/dxl/crc/
uint16_t update_crc(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size)
{
	uint16_t i, j;
	uint16_t crc_table[256] = {
		0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
		0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
		0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
		0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
		0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
		0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
		0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
		0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
		0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
		0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
		0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
		0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
		0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
		0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
		0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
		0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
		0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
		0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
		0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
		0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
		0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
		0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
		0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
		0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
		0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
		0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
		0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
		0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
		0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
		0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
		0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
		0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
	};

	for (j = 0; j < data_blk_size; j++)
	{
		i = ((uint16_t)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}

	return crc_accum;
}
