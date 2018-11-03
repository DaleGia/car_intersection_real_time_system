#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sched.h>
#include "../../states.h"
#include "../../shared_data_structures.h"
#include <sys/dispatch.h>
#include <fcntl.h>
#include <share.h>

struct s_request_message
{
	struct _pulse hdr;
	int ClientID;
	char data[3];
};


int main(void)
{
	int exit_flag = 0;
	char buf[20];
	int input = -1;
	int input2 = -1;
	int input3 = -1;
	while(!exit_flag)
	{
		puts("\n");
		puts("0: Print current node data");
		puts("1: Override current node state");
		puts("2: Disable node override");
		printf("Please select an option: ");
		if(fgets(buf, sizeof(buf), stdin) != NULL)
		{
			input = atoi(buf);
		}
		switch(input)
		{
			case 0:
			{
				puts("\n");
				puts("0: Traffic lights 1");
				puts("1: Rail Crossing");
				puts("2: Traffic Lights 2");
				printf("Please select an option: ");
				if(fgets(buf, sizeof(buf), stdin) != NULL)
				{
					input2 = atoi(buf);
				}
				switch(input2)
				{
					case 0:
					{
						getTrafficState(1);
						break;
					}
					case 1:
					{
						getRailState();
						break;
					}
					case 2:
					{
						getTrafficState(2);
						break;
					}
					default:
					{
						puts("ARGUMENT ERROR 0");
					}
				}
				break;
			}
			case 1:
			{
				puts("\n");
				puts("0: Traffic lights 1");
				puts("1: Rail Crossing");
				puts("2: Traffic Lights 2");
				printf("Please select an option: ");
				if(fgets(buf, sizeof(buf), stdin) != NULL)
				{
					input2 = atoi(buf);
				}
				puts("\n");
				puts("0: EWR_NSR");
				puts("1: EWG_NSR");
				puts("2: EWR_NSG");
				puts("3: EWY_NSR");
				puts("4: EWR_NSY");
				puts("5: TRAFFIC_ERROR");
				puts("6: GATE_OPEN");
				puts("7: GATE_CLOSED");
				puts("8: RAIL_ERROR");
				printf("Please select an option: ");
				if(fgets(buf, sizeof(buf), stdin) != NULL)
				{
					input3 = atoi(buf);
				}
				struct s_override_data override_data;
				override_data.override_active_flag = ACTIVE;

				switch(input3)
				{
					case 0:
					{
				        override_data.override_new_state = EWR_NSR;
						break;
					}
					case 1:
					{
				        override_data.override_new_state = EWG_NSR;
						break;
					}
					case 2:
					{
				        override_data.override_new_state = EWR_NSG;
						break;
					}
					case 3:
					{
				        override_data.override_new_state = EWY_NSR;
						break;
					}
					case 4:
					{
				        override_data.override_new_state = EWR_NSY;
						break;
					}
					case 5:
					{
				        override_data.override_new_state = TRAFFIC_ERROR;
						break;
					}
					case 6:
					{
				        override_data.override_new_state = GATE_OPEN;
						break;
					}
					case 7:
					{
				        override_data.override_new_state = GATE_CLOSED;
						break;
					}
					case 8:
					{
				        override_data.override_new_state = RAIL_ERROR;
						break;
					}
					default:
					{
						puts("ARGUMENT ERROR 1");
					}
				}
				puts("\n");
				puts("0: Automatic Mode");
				puts("1: Sensor Mode");
				printf("Please select an option: ");
				if(fgets(buf, sizeof(buf), stdin) != NULL)
				{
					input3 = atoi(buf);
				}
				if(input3 == 0)
				{
					override_data.automatic_mode = ACTIVE;
				}
				else if(input3 == 1)
				{
					override_data.automatic_mode = INACTIVE;
				}

				switch(input2)
				{
					case 0:
					{
						if(trafficLightOverride(&override_data, 1) == -1)
						{
							puts("ERROR: trafficLightOverride");
						}
						break;
					}
					case 1:
					{
						if(railOverride(&override_data) == -1)
						{
							puts("ERROR: railOverride");
						}
						break;
					}
					case 2:
					{
						if(trafficLightOverride(&override_data) == -1)
						{
							puts("ERROR: trafficLightOverride");
						}
						break;
					}
					default:
					{
						puts("ARGUMENT ERROR 2");
					}
				}
				break;
			}
			case 2:
			{
				puts("\n");
				puts("0: Traffic lights 1");
				puts("1: Rail Crossing");
				puts("2: Traffic Lights 2");
				printf("Please select an option: ");
				if(fgets(buf, sizeof(buf), stdin) != NULL)
				{
					input2 = atoi(buf);
				}
				struct s_override_data override_data;
				override_data.override_active_flag = INACTIVE;
				override_data.automatic_mode = UNDEFINED;
				switch(input2)
				{
					case 0:
					{
				        if(trafficLightOverride(&override_data, 1) == -1)
				        {
				        	puts("ERROR: trafficLightOverride");
				        }
						break;
					}
					case 1:
					{
				        if(railOverride(&override_data) == -1)
				        {
				        	puts("ERROR: railOverride");
				        }
						break;
					}
					case 2:
					{
				        if(trafficLightOverride(&override_data) == -1)
				        {
				        	puts("ERROR: trafficLightOverride");
				        }
						break;
					}
					default:
					{
						puts("ARGUMENT ERROR 3");
					}
				}
				break;
			}
			case 3:
			{
				exit_flag = 1;
				break;
			}
			default:
			{
				puts("ARGUMENT ERROR");
			}
		}
	}
}

int trafficLightOverride(void* data, int TL)
{

	const char *attach_point;
	switch(TL){
		case 1:
			attach_point = TRAFFIC_1_OVERRIDE_ATTACH_POINT;
			break;
		case 2:
			attach_point = TRAFFIC_2_OVERRIDE_ATTACH_POINT;
			break;
	}

	int server_connected_flag = 0;
	int server_coid;

	struct s_override_data *override_data = (struct s_override_data*) data;

		/*
		 * This is creating a message client. This message client will send data to the two
		 * Traffic lights.
		 */
	override_data->ClientID = 200; // unique number for this client (optional)

	if(server_connected_flag == 0)
	{
		printf("Trying to connect to server named: %s\n", attach_point);

		if((server_coid = name_open(attach_point, 0)) == -1)
		{
			server_connected_flag = 0;
			printf("\n    ERROR, could not connect to %s!\n\n", attach_point);
			return -1;
		}
		else
		{
			server_connected_flag = 1;
			printf("Connection established to: %s\n", attach_point);

			// We would have pre-defined data to stuff here
			override_data->hdr.type = 0x00;
			override_data->hdr.subtype = 0x00;



			struct s_server_reply_data reply;


			printf("Client (ID:%d), sending data packet: %s \n", override_data->ClientID, &override_data);
			fflush(stdout);
			if(MsgSend(server_coid, override_data, sizeof(struct s_override_data), &reply, sizeof(reply)) == -1)
			{
				printf("Error data NOT sent to server\n");
				return -1;
				// maybe we did not get a reply from the server
			}
			else
			{
				// now process the reply
				printf("Reply is: %s\n", reply.data);
			}
		}
	}
	name_close(server_coid);
	return 1;
}

int getTrafficState(int TL)
{
	const char *attach_point;
	switch(TL){
		case 1:
			attach_point = TRAFFIC_1_DATA_SHARE_ATTACH_POINT;
			break;
		case 2:
			attach_point = TRAFFIC_2_DATA_SHARE_ATTACH_POINT;
			break;
	}

	int server_connected_flag = 0;
	int server_coid;

	struct s_request_message request;
	request.ClientID = 300;
	request.hdr.type = 0x00;
	request.hdr.code = 0x00;
	request.data[0] = 'R';
	request.data[1] = 'Q';
	request.data[2] = '\n';


	struct s_traffic_message_data traffic_message;

		/*
		 * This is creating a message client. This message client will send data to the two
		 * Traffic lights.
		 */

	if(server_connected_flag == 0)
	{
		//printf("Trying to connect to server named: %s\n", attach_point);

		if((server_coid = name_open(attach_point, 0)) == -1)
		{
			server_connected_flag = 0;
			printf("\n    ERROR, could not connect to %s!\n\n", attach_point);
			return -1;
		}
		else
		{
			server_connected_flag = 1;
			//printf("Connection established to: %s\n", attach_point);

			// We would have pre-defined data to stuff here


			//printf("Client (ID:%d), sending data packet: %s \n", request.ClientID, request.data);
			fflush(stdout);
			if(MsgSend(server_coid, &request, sizeof(request), &traffic_message, sizeof(traffic_message)) == -1)
			{
				printf("Error data NOT sent to server\n");
				return -1;
				// maybe we did not get a reply from the server
			}
			else
			{
				// now process the reply
				printTrafficStatus(traffic_message, TL);
			}
		}
	}
	name_close(server_coid);
	return 1;
}

int railOverride(void* data)
{

	int server_connected_flag = 0;
	int server_coid;

	struct s_override_data *override_data = (struct s_override_data*) data;

		/*
		 * This is creating a message client. This message client will send data to the two
		 * Traffic lights.
		 */
	override_data->ClientID = 200; // unique number for this client (optional)

	if(server_connected_flag == 0)
	{
		printf("Trying to connect to server named: %s\n", RAIL_OVERRIDE_ATTACH_POINT);

		if((server_coid = name_open(RAIL_OVERRIDE_ATTACH_POINT, 0)) == -1)
		{
			server_connected_flag = 0;
			printf("\n    ERROR, could not connect to %s!\n\n", RAIL_OVERRIDE_ATTACH_POINT);
			return -1;
		}
		else
		{
			server_connected_flag = 1;
			printf("Connection established to: %s\n", RAIL_OVERRIDE_ATTACH_POINT);

			// We would have pre-defined data to stuff here
			override_data->hdr.type = 0x00;
			override_data->hdr.subtype = 0x00;



			struct s_server_reply_data reply;


			printf("Client (ID:%d), sending data packet: %s \n", override_data->ClientID, &override_data);
			fflush(stdout);
			if(MsgSend(server_coid, override_data, sizeof(struct s_override_data), &reply, sizeof(reply)) == -1)
			{
				printf("Error data NOT sent to server\n");
				// maybe we did not get a reply from the server
				return -1;
			}
			else
			{
				// now process the reply
				printf("Reply is: %s\n", reply.data);
			}
		}
	}
	name_close(server_coid);
	return 1;
}


int getRailState()
{


	int server_connected_flag = 0;
	int server_coid;

	struct s_request_message request;
	request.ClientID = 300;
	request.hdr.type = 0x00;
	request.hdr.code = 0x00;
	request.data[0] = 'R';
	request.data[1] = 'Q';
	request.data[2] = '\n';


	struct s_rail_message_data rail_message;

		/*
		 * This is creating a message client. This message client will send data to the two
		 * Traffic lights.
		 */

	if(server_connected_flag == 0)
	{
		//printf("Trying to connect to server named: %s\n", RAIL_DATA_SHARE_ATTACH_POINT);

		if((server_coid = name_open(RAIL_DATA_SHARE_ATTACH_POINT, 0)) == -1)
		{
			server_connected_flag = 0;
			printf("\n    ERROR, could not connect to %s!\n\n", RAIL_DATA_SHARE_ATTACH_POINT);
			return -1;
		}
		else
		{
			server_connected_flag = 1;
			//printf("Connection established to: %s\n", RAIL_DATA_SHARE_ATTACH_POINT);

			// We would have pre-defined data to stuff here


			//printf("Client (ID:%d), sending data packet: %s \n", request.ClientID, request.data);
			fflush(stdout);
			if(MsgSend(server_coid, &request, sizeof(request), &rail_message, sizeof(rail_message)) == -1)
			{
				printf("Error data NOT sent to server\n");
				return -1;
				// maybe we did not get a reply from the server
			}
			else
			{
				// now process the reply
				printRailStatus(rail_message);
			}
		}
	}
	name_close(server_coid);
	return 1;
}


void printTrafficStatus(struct s_traffic_message_data traffic_message, int TL)
{
	printf("\nTraffic Light %d Status: ", TL);
	printTrafficState(traffic_message.current_state);
	printf("North Car Sensor: %d\n", traffic_message.north_car_sensor);
	printf("South Car Sensor: %d\n", traffic_message.south_car_sensor);
	printf("East Car Sensor: %d\n", traffic_message.east_car_sensor);
	printf("West Car Sensor: %d\n", traffic_message.west_car_sensor);
	printf("North-South Pedestrain Button: %d\n", traffic_message.north_south_pedestrain_sensor);
	printf("East-West Pedestrain Button: %d\n", traffic_message.east_west_pedestrain_sensor);
	if(traffic_message.automatic_mode == ACTIVE){
		printf("Automatic mode active\n");
	}
	else if(traffic_message.automatic_mode == INACTIVE){
		printf("Sensor mode active\n");
	}
}

void printTrafficState(int state)
{
	time_t time_of_day;
	time_of_day = time(NULL);
	printf("%s", ctime(&time_of_day));
	printf("Current State: ");

	switch(state)
	{
		case EWR_NSR_INIT:
		{
			printf("EWR_NSR_INIT");
			break;
		}
		case EWR_NSR:
		{
			printf("EWR_NSR");
			break;
		}
		case EWG_NSR:
		{
			printf("EWG_NSR");
			break;
		}
		case EWR_NSG:
		{
			printf("EWR_NSG");
			break;
		}
		case EWY_NSR:
		{
			printf("EWY_NSR");
			break;
		}
		case EWR_NSY:
		{
			printf("EWR_NSY");
			break;
		}
		case TRAFFIC_ERROR:
		{
			printf("TRAFFIC_ERROR");
			break;
		}
	}
	printf("\n");
}

void printRailStatus(struct s_rail_message_data rail_message)
{
	printf("Rail Status: ");
	time_t time_of_day;
	time_of_day = time(NULL);
	printf("%s", ctime(&time_of_day));
	if(rail_message.train_present_flag == PRESENT){
		printf("TRAIN PRESENT\n");
	}
	else if(rail_message.train_present_flag == NOT_PRESENT){
		printf("TRAIN NOT PRESENT\n");
	}
	printf("east-north Train Sensor: %d\n", rail_message.east_north_train_sensor);
	printf("east-south Train Sensor: %d\n", rail_message.east_south_train_sensor);
	printf("west_north Train Sensor: %d\n", rail_message.west_north_train_sensor);
	printf("west_south Train Sensor: %d\n\n", rail_message.west_south_train_sensor);
}
