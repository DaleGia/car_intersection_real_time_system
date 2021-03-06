#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include "../../states.h"
#include "../../shared_data_structures.h"
#include <sys/dispatch.h>
#include <fcntl.h>
#include <share.h>

#define RAIL_OVERRIDE_ATTACH_POINT_NAME "RAIL_OVERRIDE_ATTACH_POINT"
#define RAIL_DATA_SHARE_ATTACH_POINT_NAME "RAIL_DATA_SHARE_ATTACH_POINT"

#define RAIL_CROSSING_NAME "X1"
#define RAIL_TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL
#define POLL_TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL

#define DEFAULT_RED_TIMING 1;
#define DEFAULT_POLL_TIMING_NS 500000000;

#define RAIL_I_EAST_NORTH 0
#define RAIL_I_WEST_NORTH 1
#define RAIL_I_EAST_SOUTH 2
#define RAIL_I_WEST_SOUTH 3

#define RAIL_O_EAST_NORTH 0
#define RAIL_O_WEST_NORTH 1
#define RAIL_O_EAST_SOUTH 2
#define RAIL_O_WEST_SOUTH 3
/*
 * Sensors stuff
 */
#include <hw/inout.h>      // for in32() and out32();
#include <sys/mman.h>      // for mmap_device_io();
#include <stdint.h>		   // for unit32 types
#include <string.h>
#include <sys/procmgr.h>

#define PIO_SIZE			0x0000001F
#define PIO_SIZE_Switches	0x0000000F
#define LEDs_BASE			0xFF200000  //(LEDs - 8 bits wide - output only)
#define GPIO_1_BASE			0xFF200020  //(GPIO_1 - JP7 header - 16 bits wide - Input only)
#define GPIO_0_BASE			0xFF200040  //(GPIO_0 - JP1 header - 16 bits wide - Output only)
#define SWITCHES_BASE		0xFF200060  //(Switches - 4 bits wide - Inputs only)


struct s_io_id
{
	uintptr_t gpio_LEDs;
	uintptr_t gpio_0_outputs;
	uintptr_t gpio_1_inputs;
	uintptr_t Switches_inputs;
};

struct s_rail_data
{
	struct s_override_data override_data;
	struct s_rail_message_data message_data;
	pthread_rwlock_t rwlock;
};

struct s_signal_msg
{
	struct _pulse pulse;
};

struct s_request_message
{
	   struct _pulse hdr;  // Our real data comes after this header
	   int ClientID;
       char data[3]; // Message we send back to clients to tell them the messages was processed correctly.
};

struct s_rail_timing_event
{
	pthread_rwlock_t rwlock;

	struct sigevent rail_timer_event;
	struct itimerspec instant_timer_spec;
	struct itimerspec red_timer_spec;
	timer_t timer_id;
	int channel_id;
	int receive_id;
	struct s_signal_msg msg;

	struct sigevent poll_timer_event;
	struct itimerspec poll_timer_spec;
	timer_t poll_timer_id;
	int poll_channel_id;
	int poll_receive_id;
	struct s_signal_msg poll_msg;
};

struct s_rail_control
{
	struct s_rail_timing_event rail_timing;
	struct s_rail_data rail_data;
	struct s_io_id io_id;
};

void initialiseRailCrossingData(struct s_rail_data *rdata);
int initialiseTimers(struct s_rail_timing_event *timing_data);
void *railCrossingControl(void* rail_control_data);
void railOverrideHandler(struct s_rail_data *rdata);
void updateLights(struct s_rail_control *rdata);
void* sensorPoll(void *data);
void printState(enum e_STATE state);
void sendTrafficLightMessage(struct s_rail_message_data *rail_data);
void* connectClientToServer(void* data);
void* overrideServer(void *data);
void* dataShareServer(void *data);


int initIO(struct s_rail_control *data);
void unmapIO(struct s_rail_control *data);

int server_coid_1;
int server_coid_2;
int server_1_connected_flag = 0;
int server_2_connected_flag = 0;

int main(void)
{

	// Puts it all in one nice neat struct for the thread stuff.
	struct s_rail_control  rail_control_data;
	/*
	 * These structure will be updated as sensor values are updated.
	 *	Initialises all of the traffic data to initial values.	 *
	 */

	//struct s_rail_data rail_data;
	initialiseRailCrossingData(&rail_control_data.rail_data);
	/*
	 * This holds all the timing event and signals for he traffic light.
	 * The following code initialises all of the timing and signal stuff.
	 */

	//struct s_traffic_timing_event traffic_timing;
	if(initialiseTimers(&rail_control_data.rail_timing) < 0)
	{
		perror (NULL);
		exit (EXIT_FAILURE);
	}

	if(!initIO(&rail_control_data))
	{
		puts("Failed to initiate IO, exiting main...");
		return EXIT_SUCCESS;
	}


	// Creates thread and thread attributes for traffic lights
	pthread_t  rail_control_thread;
	pthread_attr_t rail_control_thread_attr;
	struct sched_param rail_control_thread_param;
	pthread_attr_init(&rail_control_thread_attr);
	rail_control_thread_param.sched_priority = 10;
	pthread_attr_setschedparam(&rail_control_thread_attr, &rail_control_thread_param);

	// Creates thread and thread attributes for polling
	pthread_t  poll_thread;
	pthread_attr_t poll_thread_attr;
	struct sched_param poll_thread_param;
	pthread_attr_init(&poll_thread_attr);
	poll_thread_param.sched_priority = 5;
	pthread_attr_setschedparam(&poll_thread_attr, &poll_thread_param);

	// Creates thread and thread attributes for client
	pthread_t  client_thread;
	pthread_attr_t client_thread_attr;
	struct sched_param client_thread_param;
	pthread_attr_init(&client_thread_attr);
	client_thread_param.sched_priority = 4;
	pthread_attr_setschedparam(&client_thread_attr, &client_thread_param);

	// Creates thread and thread attributes for client
	pthread_t  override_thread;
	pthread_attr_t override_thread_attr;
	struct sched_param override_thread_param;
	pthread_attr_init(&override_thread_attr);
	override_thread_param.sched_priority = 10;
	pthread_attr_setschedparam(&override_thread_attr, &override_thread_param);

	// Creates thread attributes for data share message server
	pthread_t  data_share_server_thread;
	pthread_attr_t data_share_server_thread_attr;
	struct sched_param data_share_server_thread_param;
	pthread_attr_init(&data_share_server_thread_attr);
	data_share_server_thread_param.sched_priority = 5;
	pthread_attr_setschedparam(&data_share_server_thread_attr, &data_share_server_thread_param);



	// Starts traffic light control thread.
	pthread_create(&rail_control_thread, &rail_control_thread_attr, railCrossingControl, &rail_control_data);
	// starts poll thread.
	pthread_create(&poll_thread, &poll_thread_attr, sensorPoll, &rail_control_data);
	// Starts client thread
	//pthread_create(&client_thread, &client_thread_attr, connectClientToServer, &rail_control_data.rail_data.message_data);
	// Starts override thread
	pthread_create(&override_thread, &override_thread_attr, overrideServer, &rail_control_data);
	//data share message thread
	pthread_create(&data_share_server_thread, &data_share_server_thread_attr, dataShareServer, &rail_control_data);



	void* ret;
	pthread_join(rail_control_thread, &ret);
    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server_coid_1);
    name_close(server_coid_2);

	return EXIT_SUCCESS;
}

void* connectClientToServer(void* data)
{
	struct s_rail_message_data *message_data = (struct s_rail_message_data*) data;

	while(1)
	{
		/*
		 * This is creating a message client. This message client will send data to the two
		 * Traffic lights.
		 */
		message_data->ClientID = 100; // unique number for this client (optional)
		if(server_1_connected_flag == 0)
		{
			printf("Trying to connect to server named: %s\n", TRAFFIC_1_RAIL_ATTACH_POINT);

			if((server_coid_1 = name_open(TRAFFIC_1_RAIL_ATTACH_POINT, 0)) == -1)
			{
				server_1_connected_flag = 0;
				printf("\n    ERROR, could not connect to server!\n\n");
			}
			else
			{
				server_1_connected_flag = 1;
				printf("Connection established to: %s\n", TRAFFIC_1_RAIL_ATTACH_POINT);

				// We would have pre-defined data to stuff here
				message_data->hdr.type = 0x00;
				message_data->hdr.subtype = 0x00;
			}
		}
		if(server_2_connected_flag == 0)
		{
			printf("Trying to connect to server named: %s\n", TRAFFIC_2_RAIL_ATTACH_POINT);
			if((server_coid_2 = name_open(TRAFFIC_2_RAIL_ATTACH_POINT, 0)) == -1)
			{
				server_2_connected_flag = 0;
				printf("\n    ERROR, could not connect to server!\n\n");
			}
			else
			{
				server_2_connected_flag = 1;
				printf("Connection established to: %s\n", TRAFFIC_2_RAIL_ATTACH_POINT);

				// We would have pre-defined data to stuff here
				message_data->hdr.type = 0x00;
				message_data->hdr.subtype = 0x00;
			}
		}
		sleep(3);
	}
}

void sendTrafficLightMessage(struct s_rail_message_data *rail_data)
{
	printf("\nCurrent State: ");
	printState(rail_data->current_state);
	// the data we are sending is in msg.data
	struct s_server_reply_data reply;

    //rail_data->hdr.type = 0x00;
    //rail_data->hdr.subtype = 0x00;

	if(server_1_connected_flag)
	{
		printf("Client (ID:%d), sending data packet: %s \n", rail_data->ClientID, &rail_data);
		fflush(stdout);
		if(MsgSend(server_coid_1, rail_data, sizeof(struct s_rail_message_data), &reply, sizeof(reply)) == -1)
		{
			printf("Error data NOT sent to server\n");
			// maybe we did not get a reply from the server
		}
		else
		{
			// now process the reply
			printf("Reply is: %s\n", reply.data);
		}
	}
	if(server_2_connected_flag)
	{
		printf("Client (ID:%d), sending data packet with the integer value: %d \n", rail_data->ClientID, rail_data);
		fflush(stdout);
		if(MsgSend(server_coid_2, rail_data, sizeof(struct s_rail_message_data), &reply, sizeof(reply)) == -1)
		{
			printf("Error data NOT sent to server\n");
			// maybe we did not get a reply from the server
		}
		else
		{
			// now process the reply
			printf("Reply is: '%s'\n", reply.data);
		}
	}
}

void initialiseRailCrossingData(struct s_rail_data *rdata)
{
	struct s_rail_data *rail_data = (struct s_rail_data*) rdata;

	rail_data->message_data.lights_red_length = 10;

	rail_data->override_data.override_active_flag = 0;
	rail_data->override_data.override_new_state = RAIL_ERROR;

	rail_data->message_data.train_present_flag = NOT_PRESENT;
	rail_data->message_data.current_state = GATE_CLOSED_INIT;

	rail_data->message_data.east_north_train_sensor = INACTIVE;
	rail_data->message_data.east_south_train_sensor = INACTIVE;
	rail_data->message_data.west_north_train_sensor = INACTIVE;
	rail_data->message_data.west_south_train_sensor = INACTIVE;
}


int initialiseTimers(struct s_rail_timing_event *timing_data)
{
	struct s_rail_timing_event *rail_timing = (struct s_rail_timing_event*) timing_data;
	// Creates communication channel
	rail_timing->channel_id = ChannelCreate(0);
	rail_timing->poll_channel_id = ChannelCreate(0);
	rail_timing->rail_timer_event.sigev_notify = SIGEV_PULSE;
	rail_timing->poll_timer_event.sigev_notify = SIGEV_PULSE;
	// create a connection back to ourselves for the timer to send the pulse on
	rail_timing->rail_timer_event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, rail_timing->channel_id, _NTO_SIDE_CHANNEL, 0);
	rail_timing->poll_timer_event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, rail_timing->poll_channel_id, _NTO_SIDE_CHANNEL, 0);
	if(rail_timing->rail_timer_event.sigev_coid == -1)
	{
	   printf("%s:  rail timing couldn't ConnectAttach to self!\n", RAIL_CROSSING_NAME);
	   return -1;
	}

	if(rail_timing->poll_timer_event.sigev_coid == -1)
	{
	   printf("%s:  poll timing couldn't ConnectAttach to self!\n", RAIL_CROSSING_NAME);
	   return -1;
	}
		// The following line might be needed?
	//event.sigev_priority = getprio(0);
	rail_timing->rail_timer_event.sigev_code = RAIL_TIMER_PULSE_CODE;
	rail_timing->poll_timer_event.sigev_code = POLL_TIMER_PULSE_CODE;
	//create the timer, binding it to the event
	if(timer_create(CLOCK_REALTIME, &rail_timing->rail_timer_event, &rail_timing->timer_id) == -1)
	{
	   printf("%s:  couldn't create a rail timer\n", RAIL_CROSSING_NAME);
	   return -1;
	}

	if(timer_create(CLOCK_REALTIME, &rail_timing->poll_timer_event, &rail_timing->poll_timer_id) == -1)
	{
	   printf("%s:  couldn't create a poll timer\n", RAIL_CROSSING_NAME);
	   return -1;
	}

	rail_timing->red_timer_spec.it_value.tv_sec = DEFAULT_RED_TIMING;
	rail_timing->red_timer_spec.it_value.tv_nsec = 0;
	rail_timing->red_timer_spec.it_interval.tv_sec = DEFAULT_RED_TIMING;
	rail_timing->red_timer_spec.it_interval.tv_nsec = 0;


	rail_timing->instant_timer_spec.it_value.tv_sec = 0;
	rail_timing->instant_timer_spec.it_value.tv_nsec = 1000;
	rail_timing->instant_timer_spec.it_interval.tv_sec = 0;
	rail_timing->instant_timer_spec.it_interval.tv_nsec = 1000;

	rail_timing->poll_timer_spec.it_value.tv_sec = 0;
	rail_timing->poll_timer_spec.it_value.tv_nsec = DEFAULT_POLL_TIMING_NS;
	rail_timing->poll_timer_spec.it_interval.tv_sec = 0;
	rail_timing->poll_timer_spec.it_interval.tv_nsec = DEFAULT_POLL_TIMING_NS;
	return 0;
}

void startTimer(timer_t timer, struct itimerspec spec)
{
	timer_settime(timer, 0, &spec, NULL);
}

void *railCrossingControl(void *data)
{
	struct s_rail_control *rail_control_data = (struct s_rail_control*) data;
	struct s_rail_data *rail_data = &rail_control_data->rail_data;
	struct s_rail_timing_event *timing_data = &rail_control_data->rail_timing;
	// Do first timer.
	startTimer(timing_data->timer_id, timing_data->instant_timer_spec);
	puts("railCrossingControl: first timer started");
	while(1)
	{

		// Wait for for message timing pulse
		timing_data->receive_id = MsgReceive(timing_data->channel_id, &timing_data->msg, sizeof(timing_data->msg), NULL);

		// Test if the message came from this process
		if(timing_data->receive_id == 0)
		{
			// Check we got a pulse
			if(timing_data->msg.pulse.code == RAIL_TIMER_PULSE_CODE)
			{
				// All is good. We can try and calculate the traffic
				// lights now.
				pthread_rwlock_rdlock(&rail_data->rwlock);
				updateLights(rail_control_data);
				/*
				 *	Handles a central control override situation.
				 */
				/*&
				if(rail_data->override_data.override_active_flag)
				{
					puts("railCrossingControl: Central overide detected");
					rail_data->message_data.previous_state = rail_data->message_data.current_state;
					railOverrideHandler(&rail_control_data->rail_data);
					startTimer(timing_data->timer_id, timing_data->red_timer_spec);
				}
				else
				{
				*/
					switch(rail_data->message_data.current_state)
					{
						case GATE_CLOSED_INIT:
						{
							if(rail_data->message_data.previous_state != GATE_CLOSED_INIT)
							{
								sendTrafficLightMessage(&rail_data->message_data);
							}
							rail_data->message_data.previous_state = rail_data->message_data.current_state;
							rail_data->message_data.current_state = GATE_CLOSED;
							//Sets timer to instantly call an output state update.
							startTimer(timing_data->timer_id, timing_data->red_timer_spec);
							break;
						}
						case GATE_OPEN:
						{

							/*
							 * 	Handles if train is coming and boom gates are closed.
							 */
							if(rail_data->message_data.previous_state != GATE_OPEN)
							{
								sendTrafficLightMessage(&rail_data->message_data);
							}
							rail_data->message_data.previous_state = rail_data->message_data.current_state;
							if(rail_data->message_data.train_present_flag == PRESENT)
							{
								rail_data->message_data.current_state = GATE_CLOSED;
							}

							startTimer(timing_data->timer_id, timing_data->red_timer_spec);
							break;
						}
						case GATE_CLOSED:
						{
							if(rail_data->message_data.previous_state != GATE_CLOSED)
							{
								sendTrafficLightMessage(&rail_data->message_data);
							}
							rail_data->message_data.previous_state = rail_data->message_data.current_state;
							if(rail_data->message_data.train_present_flag == NOT_PRESENT)
							{
								rail_data->message_data.current_state = GATE_OPEN;
							}

							startTimer(timing_data->timer_id, timing_data->red_timer_spec);
							break;
						}
						case RAIL_ERROR:
						{
							if(rail_data->message_data.previous_state != RAIL_ERROR)
							{
								sendTrafficLightMessage(&rail_data->message_data);
							}
							rail_data->message_data.previous_state = rail_data->message_data.current_state;
							rail_data->message_data.current_state = GATE_CLOSED;
							startTimer(timing_data->timer_id, timing_data->red_timer_spec);
							break;
						}
						default:
						{
							break;
						}
					}
				//}
				pthread_rwlock_unlock(&rail_data->rwlock);
			}
		}
	}
}

void railOverrideHandler(struct s_rail_data *tdata)
{
	struct s_rail_data *rail_data = (struct s_rail_data*) tdata;
	rail_data->message_data.previous_state = rail_data->message_data.current_state;
	switch(rail_data->override_data.override_new_state)
	{
		case GATE_CLOSED:
		{
			rail_data->message_data.current_state = GATE_CLOSED;
			break;
		}
		case GATE_OPEN:
		{
			rail_data->message_data.current_state = GATE_OPEN;
			break;
		}
		default:
		{
			rail_data->message_data.current_state = RAIL_ERROR;
			break;
		}
	}
}

void updateLights(struct s_rail_control *tdata)
{
	struct s_rail_control *rail_control_data = (struct s_rail_control*) tdata;
	struct s_rail_data *rail_data = &rail_control_data->rail_data;
	struct s_io_id *io_id = &rail_control_data->io_id;
	volatile uint32_t	led = 0;
	switch(rail_data->message_data.current_state)
	{
		case GATE_CLOSED_INIT:
		{
			led |= (1<<RAIL_O_EAST_NORTH) | (1<<RAIL_O_WEST_NORTH) | (1<<RAIL_O_EAST_SOUTH) | (1<<RAIL_O_WEST_SOUTH);
			break;
		}
		case GATE_OPEN:
		{
			led = 0;
			break;
		}
		case GATE_CLOSED:
		{
			led |= (1<<RAIL_O_EAST_NORTH) | (1<<RAIL_O_WEST_NORTH) | (1<<RAIL_O_EAST_SOUTH) | (1<<RAIL_O_WEST_SOUTH);
			break;
		}
		case RAIL_ERROR:
		{
			led |= (1<<RAIL_O_EAST_NORTH) | (1<<RAIL_O_WEST_NORTH) | (1<<RAIL_O_EAST_SOUTH) | (1<<RAIL_O_WEST_SOUTH);
			break;
		}
		default:
		{
			break;
		}
	}

	out32(io_id->gpio_LEDs,led);
//	printf("output = %#04X\n\n",output);
//	out32(io_id->gpio_0_outputs, output);

}

void* sensorPoll(void *data)
{
	struct s_rail_control *rail_control_data = (struct s_rail_control*) data;
	struct s_rail_data *rail_data = (struct s_rail_data*) &rail_control_data->rail_data;
	struct s_rail_timing_event *timing_data = &rail_control_data->rail_timing;
	struct s_io_id *io_id = &rail_control_data->io_id;
	volatile uint32_t	switches = 0;


	// start first timer.
	startTimer(timing_data->poll_timer_id, timing_data->poll_timer_spec);
	int sensor_change_flag = 0;
	while(1)
	{
		// Wait for polling timer.
		timing_data->poll_receive_id = MsgReceive(timing_data->poll_channel_id, &timing_data->poll_msg, sizeof(timing_data->poll_msg), NULL);
		// Make sure it is the correct id.
		if(timing_data->poll_receive_id == 0)
		{
			// Check we got a pulse
			if(timing_data->poll_msg.pulse.code == POLL_TIMER_PULSE_CODE)
			{
				// All is good. We can try and get the sensor values you.
				pthread_rwlock_rdlock(&rail_data->rwlock);

			 	switches  = in32(io_id->Switches_inputs);
			 	rail_data->message_data.train_present_flag = NOT_PRESENT;


			 	if((switches >> RAIL_I_EAST_NORTH) & 1)
				{
			 	//	puts("Train east north sensor active");
			 		if(rail_data->message_data.east_north_train_sensor != ACTIVE)
			 		{
			 			rail_data->message_data.east_north_train_sensor = ACTIVE;
			 			sensor_change_flag = 1;
			 		}
			 		rail_data->message_data.train_present_flag = PRESENT;
			 	}
			 	else
			 	{
			 		if(rail_data->message_data.east_north_train_sensor == ACTIVE)
			 		{
			 			rail_data->message_data.east_north_train_sensor = INACTIVE;
			 			sensor_change_flag = 1;
			 		}
			 	}
			 	if((switches >> RAIL_I_EAST_SOUTH) & 1)
			 	{
			 		if(rail_data->message_data.east_south_train_sensor != ACTIVE)
			 		{
						rail_data->message_data.east_south_train_sensor = ACTIVE;
			 			sensor_change_flag = 1;
			 		}
					rail_data->message_data.train_present_flag = PRESENT;
			 	}
			 	else
			 	{
			 		if(rail_data->message_data.east_south_train_sensor == ACTIVE)
			 		{
			 			rail_data->message_data.east_south_train_sensor = INACTIVE;
			 			sensor_change_flag = 1;
			 		}
			 	}
			 	if((switches >> RAIL_I_WEST_NORTH) & 1)
				{
			 		if(rail_data->message_data.west_north_train_sensor != ACTIVE)
			 		{
						rail_data->message_data.west_north_train_sensor = ACTIVE;
			 			sensor_change_flag = 1;
					}
					rail_data->message_data.train_present_flag = PRESENT;
				}
			 	else
			 	{
			 		if(rail_data->message_data.west_north_train_sensor == ACTIVE)
			 		{
			 			rail_data->message_data.west_north_train_sensor = INACTIVE;
			 			sensor_change_flag = 1;
			 		}
			 	}
			 	if((switches >> RAIL_I_WEST_SOUTH) & 1)
				{
			 		if(rail_data->message_data.west_south_train_sensor != ACTIVE)
			 		{
						rail_data->message_data.west_south_train_sensor = ACTIVE;
			 			sensor_change_flag = 1;
			 		}
					rail_data->message_data.train_present_flag = PRESENT;
				}
			 	else
			 	{
			 		if(rail_data->message_data.west_south_train_sensor == ACTIVE)
			 		{
			 			rail_data->message_data.west_south_train_sensor = INACTIVE;
			 			sensor_change_flag = 1;
			 		}
			 	}
				pthread_rwlock_unlock(&rail_data->rwlock);
				if(sensor_change_flag)
				{
					sendTrafficLightMessage(&rail_data->message_data);
					sensor_change_flag = 0;
				}
				//sleep(1);
				startTimer(timing_data->poll_timer_id, timing_data->poll_timer_spec);
			}
		}
	}
}

void* overrideServer(void *data)
{

	puts("overrideServer thread started");

    name_attach_t *attach;
    struct s_rail_control *rail_control = (struct s_rail_control*) data;
    struct s_rail_data *rail_data = (struct s_rail_data*) &rail_control->rail_data;
    struct s_override_data *override_data = (struct s_override_data*) &rail_data->override_data;

   // Create a local name (/dev/name/...)
   if ((attach = name_attach(NULL, RAIL_OVERRIDE_ATTACH_POINT_NAME, 0)) == NULL)
   {
       printf("\nFailed to name_attach on RAIL_OVERRIDE_ATTACH_POINT: %s \n", RAIL_OVERRIDE_ATTACH_POINT_NAME);
       printf("\n Possibly another server with the same name is already running !\n");
       pthread_exit(EXIT_SUCCESS);
   }

   printf("overrideServer Listening for Clients on RAIL_OVERRIDE_ATTACH_POINT: %s \n", RAIL_OVERRIDE_ATTACH_POINT_NAME);

   int rcvid=0;  		// no message received yet
   int Stay_alive=1, living=0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)


   struct s_server_reply_data replymsg;
   replymsg.hdr.type = 0x01;
   replymsg.hdr.subtype = 0x00;

   replymsg.data[0] = 'O';
   replymsg.data[1] = 'K';
   replymsg.data[2] = '\0';

   living =1;
   while (living)
   {
	   // Do your MsgReceive's here now with the chid
       rcvid = MsgReceive(attach->chid, override_data, sizeof(struct s_override_data), NULL);

       if (rcvid == -1)  // Error condition, exit
       {
           printf("\nFailed to MsgReceive\n");
       }

       // did we receive a Pulse or message?
       // for Pulses:
       if (rcvid == 0)  //  Pulse received, work out what type
       {
           switch (override_data->hdr.code)
           {
			   case _PULSE_CODE_DISCONNECT:
					// A client disconnected all its connections by running
					// name_close() for each name_open()  or terminated
				   if( Stay_alive == 0)
				   {
					   //ConnectDetach(msg.hdr.scoid);
					   printf("\nOverrideServer was told to Detach from ClientID:%d ...\n", override_data->ClientID);
					   living = 0; // kill while loop
					   continue;
				   }
				   else
				   {
					   //printf("\nOverrideServer received Detach pulse from ClientID:%d but rejected it ...\n", override_data->ClientID);
				   }
				   break;

			   case _PULSE_CODE_UNBLOCK:
					// REPLY blocked client wants to unblock (was hit by a signal
					// or timed out).  It's up to you if you reply now or later.
				   printf("\nOverrideServer got _PULSE_CODE_UNBLOCK");
				   break;

			   case _PULSE_CODE_COIDDEATH:  // from the kernel
				   printf("\nOverrideServer got _PULSE_CODE_COIDDEATH");
				   break;

			   case _PULSE_CODE_THREADDEATH: // from the kernel
				   printf("\nOverrideServer got _PULSE_CODE_THREADDEATH");
				   break;

			   default:
				   // Some other pulse sent by one of your processes or the kernel
				   printf("\nOverrideServer got some other pulse");
				   break;

           }
           continue;// go back to top of while loop
       }

       // for messages:
       if(rcvid > 0) // if true then A message was received
       {

		   // If the Global Name Service (gns) is running, name_open() sends a connect message. The server must EOK it.
		   if (override_data->hdr.type == _IO_CONNECT)
		   {
			   MsgReply( rcvid, EOK, NULL, 0 );
			   printf("\n gns service is running....");
			   continue;	// go back to top of while loop
		   }
		   // Some other I/O message was received; reject it
		   if (override_data->hdr.type > _IO_BASE && override_data->hdr.type <= _IO_MAX )
		   {
			   MsgError( rcvid, ENOSYS );
			   printf("\n OverrideServer received and IO message and rejected it....");
			   continue;	// go back to top of while loop
		   }

		   // A message (presumably ours) received
		   // put your message handling code here and assemble a reply message
		   //printf("Server received data packet with value of '%d' from client (ID:%d), ", rail_message->data, rail_message->ClientID);
		   fflush(stdout);
		   if(MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg))){
			   puts("message reply error");
		   //MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
		   }
		   pthread_rwlock_rdlock(&rail_data->rwlock);
		   pthread_rwlock_rdlock(&rail_control->rail_timing. rwlock);

			if(override_data->override_active_flag)
			{
				railOverrideHandler(rail_data);
				startTimer(rail_control->rail_timing.timer_id, rail_control->rail_timing.red_timer_spec);
			}


			pthread_rwlock_unlock(&rail_data->rwlock);
			pthread_rwlock_unlock(&rail_control->rail_timing.rwlock);
       }
       else
       {
		   printf("\nERROR: Server received something, but could not handle it correctly\n");
       }
   }
   puts("server thread ended");
   // Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
   name_detach(attach, 0);
   pthread_exit(EXIT_SUCCESS);
}

int initIO(struct s_rail_control *data)
{
	struct s_rail_control *rail_control_data = (struct s_rail_control*) data;
	struct s_io_id *io_id = &rail_control_data->io_id;

	io_id->gpio_LEDs = mmap_device_io(PIO_SIZE, LEDs_BASE);
	if( !io_id->gpio_LEDs )
	{
		// An error has occurred
		perror("Can't map Control Base Module / gpio_LEDs");
		printf("Mapping IO ERROR! Main Terminated...!\n");
		return 0;
	}

	io_id->Switches_inputs = mmap_device_io(PIO_SIZE_Switches, SWITCHES_BASE);
	if( !io_id->Switches_inputs )
	{
		// An error has occurred
		perror("Can't map Control Base Module / Switches_inputs");
		printf("Mapping IO ERROR! Main Terminated...!\n");
		return 0;
	}
	return 1;
}

void unmapIO(struct s_rail_control *data)
{
	struct s_rail_control *rail_control_data = (struct s_rail_control*) data;
	struct s_io_id *io_id = &rail_control_data->io_id;

	munmap_device_io(io_id->gpio_LEDs, PIO_SIZE);
	munmap_device_io(io_id->Switches_inputs, PIO_SIZE_Switches);
}

void printState(enum e_STATE state)
{
	switch(state)
	{
		case EWR_NSR_INIT:
		{
			puts("EWR_NSR_INIT");
			break;
		}
		case EWR_NSR:
		{
			puts("EWR_NSR");
			break;
		}
		case EWG_NSR:
		{
			puts("EWG_NSR");
			break;
		}
		case EWR_NSG:
		{
			puts("EWR_NSG");
			break;
		}
		case EWY_NSR:
		{
			puts("EWY_NTrySR");
			break;
		}
		case EWR_NSY:
		{
			puts("EWR_NSY");
			break;
		}
		case TRAFFIC_ERROR:
		{
			puts("TRAFFIC_ERROR");
			break;
		}
		case GATE_CLOSED_INIT:
		{
			puts("GATE_CLOSED_INIT");
			break;
		}
		case GATE_OPEN:
		{
			puts("GATE_OPEN");
			break;
		}
		case GATE_CLOSED:
		{
			puts("GATE_CLOSED");
			break;
		}
		case RAIL_ERROR:
		{
			puts("RAIL_ERROR");
			break;
		}
	}
}








void* dataShareServer(void *data)
{
	puts("data share server thread started");


    name_attach_t *attach;
    struct s_rail_control *rail_control = (struct s_rail_control*) data;
	struct s_rail_data *rail_data = (struct s_rail_data*) &rail_control->rail_data;
	struct s_rail_message_data *rail_message_reply =  (struct s_rail_message_data*) &rail_data->message_data;

	struct s_request_message request_message;

	rail_message_reply->hdr.type = 0x01;
	rail_message_reply->hdr.subtype = 0x00;


   // Create a local name (/dev/name/...)
   if ((attach = name_attach(NULL, RAIL_DATA_SHARE_ATTACH_POINT_NAME, 0)) == NULL)
   {
       printf("\nFailed to name_attach on ATTACH_POINT: %s \n", RAIL_DATA_SHARE_ATTACH_POINT_NAME);
       printf("\n Possibly another server with the same name is already running !\n");
       pthread_exit(EXIT_SUCCESS);
   }

   printf("Server Listening for Clients on ATTACH_POINT: %s \n", RAIL_DATA_SHARE_ATTACH_POINT_NAME);

   int rcvid=0;  		// no message received yet
   int Stay_alive=1, living=0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)


   living =1;
   while (living)
   {
	   // Do your MsgReceive's here now with the chid
       rcvid = MsgReceive(attach->chid, &request_message, sizeof(struct s_request_message), NULL);

       if (rcvid == -1)  // Error condition, exit
       {
           printf("\nFailed to MsgReceive\n");
       }

       // did we receive a Pulse or message?
       // for Pulses:
       if (rcvid == 0)  //  Pulse received, work out what type
       {
           switch (request_message.hdr.code)
           {
			   case _PULSE_CODE_DISCONNECT:
					// A client disconnected all its connections by running
					// name_close() for each name_open()  or terminated
				   if( Stay_alive == 0)
				   {
					   //ConnectDetach(msg.hdr.scoid);
					   printf("\nServer was told to Detach from ClientID:%d ...\n", request_message.ClientID);
					   living = 0; // kill while loop
					   continue;
				   }
				   else
				   {
					   printf("\nServer received Detach pulse from ClientID:%d but rejected it ...\n", request_message.ClientID);
				   }
				   break;

			   case _PULSE_CODE_UNBLOCK:
					// REPLY blocked client wants to unblock (was hit by a signal
					// or timed out).  It's up to you if you reply now or later.
				   printf("\nServer got _PULSE_CODE_UNBLOCK");
				   break;

			   case _PULSE_CODE_COIDDEATH:  // from the kernel
				   printf("\nServer got _PULSE_CODE_COIDDEATH");
				   break;

			   case _PULSE_CODE_THREADDEATH: // from the kernel
				   printf("\nServer got _PULSE_CODE_THREADDEATH");
				   break;

			   default:
				   // Some other pulse sent by one of your processes or the kernel
				   printf("\nServer got some other pulse");
				   break;

           }
           continue;// go back to top of while loop
       }

       // for messages:
       if(rcvid > 0) // if true then A message was received
       {

		   // If the Global Name Service (gns) is running, name_open() sends a connect message. The server must EOK it.
		   if (request_message.hdr.type == _IO_CONNECT)
		   {
			   MsgReply( rcvid, EOK, NULL, 0 );
			   printf("\n gns service is running....");
			   continue;	// go back to top of while loop
		   }
		   // Some other I/O message was received; reject it
		   if (request_message.hdr.type > _IO_BASE && request_message.hdr.type <= _IO_MAX )
		   {
			   MsgError( rcvid, ENOSYS );
			   printf("\n Server received and IO message and rejected it....");
			   continue;	// go back to top of while loop
		   }

		   // A message (presumably ours) received
		   // put your message handling code here and assemble a reply message
		   //printf("Server received data packet with value of '%d' from client (ID:%d), ", rail_message->data, rail_message->ClientID);
		   fflush(stdout);
		   pthread_rwlock_rdlock(&rail_data->rwlock);

		   if(MsgReply(rcvid, EOK, &rail_message_reply, sizeof(struct s_rail_message_data))){
			   puts("message reply error");
		   //MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
		   }

		   pthread_rwlock_unlock(&rail_data->rwlock);

       }
       else
       {
		   printf("\nERROR: Server received something, but could not handle it correctly\n");
       }
   }
   puts("data share server thread ended");
   // Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
   name_detach(attach, 0);
   pthread_exit(EXIT_SUCCESS);
}
