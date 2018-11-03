#ifndef SHAREDDATASTRUCTURES_H
#define SHAREDDATASTRUCTURES_H

#include "states.h"
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#define TRAFFIC_1_OVERRIDE_ATTACH_POINT "/net/RMIT_BBB_v5_06/dev/name/local/TRAFFIC_1_OVERRIDE_ATTACH_POINT"
#define TRAFFIC_2_OVERRIDE_ATTACH_POINT "/net/RMIT_BBB_v5_07/dev/name/local/TRAFFIC_2_OVERRIDE_ATTACH_POINT"
#define RAIL_OVERRIDE_ATTACH_POINT "/net/cycloneVg07a/dev/name/local/RAIL_OVERRIDE_ATTACH_POINT"
#define TRAFFIC_1_DATA_SHARE_ATTACH_POINT "/net/cycloneVg07a/dev/name/local/TRAFFIC_1_DATA_SHARE_ATTACH_POINT"
#define TRAFFIC_2_DATA_SHARE_ATTACH_POINT "/net/RMIT_BBB_v5_07/dev/name/local/TRAFFIC_2_DATA_SHARE_ATTACH_POINT"
#define RAIL_DATA_SHARE_ATTACH_POINT "/net/cycloneVg07a/dev/name/local/RAIL_DATA_SHARE_ATTACH_POINT"
#define TRAFFIC_1_RAIL_ATTACH_POINT "/net/RMIT_BBB_v5_06/dev/name/local/TRAFFIC_1_RAIL_ATTACH_POINT"
#define TRAFFIC_2_RAIL_ATTACH_POINT "/net/RMIT_BBB_v5_07/dev/name/local/TRAFFIC_1_RAIL_ATTACH_POINT"

struct s_traffic_message_data
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	unsigned int north_south_green_length;
	unsigned int east_west_green_length;

	enum e_SENSOR_STATE north_car_sensor;
	enum e_SENSOR_STATE south_car_sensor;
	enum e_SENSOR_STATE east_car_sensor;
	enum e_SENSOR_STATE west_car_sensor;

	enum e_SENSOR_STATE north_south_pedestrain_sensor;
	enum e_SENSOR_STATE east_west_pedestrain_sensor;

	enum e_STATE current_state;
	enum e_STATE previous_state;

	enum e_SENSOR_STATE automatic_mode;
};

struct s_rail_message_data
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	unsigned int lights_red_length;

	enum e_SENSOR_STATE east_north_train_sensor;
	enum e_SENSOR_STATE east_south_train_sensor;
	enum e_SENSOR_STATE west_north_train_sensor;
	enum e_SENSOR_STATE west_south_train_sensor;
	enum e_SENSOR_STATE train_present_flag;
	enum e_STATE current_state;
	enum e_STATE previous_state;

};

struct s_override_data
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	unsigned char override_active_flag;
	enum e_STATE override_new_state;
	enum e_SENSOR_STATE automatic_mode;
};

struct s_server_reply_data
{
	struct _pulse hdr; // Our real data comes after this header
	int ClientID; // our data (unique id from client)
	char data[3];
};

#endif
