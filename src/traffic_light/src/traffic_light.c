#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include "../../states.h"
#include "../../shared_data_structures.h"
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <share.h>


#define TRAFFIC_O_G_NS 5
#define TRAFFIC_O_G_EW 2

#define TRAFFIC_O_Y_NS 4
#define TRAFFIC_O_Y_EW 1

#define TRAFFIC_O_RED_N_S 3
#define TRAFFIC_O_R_EW 0

#define PED_O_R_EW 6
#define PED_O_R_NS 7

#define TRAFFIC_I_N 0
#define TRAFFIC_I_S 1
#define TRAFFIC_I_E 2
#define TRAFFIC_I_W 3

#define PED_I_EW 5
#define PED_I_NS 4

#define TRAFFIC_LIGHT_NAME "T1"
#define TRAFFIC_TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL
#define POLL_TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL

#define DEFAULT_GREEN_TIMING 10;
#define DEFAULT_YELLOW_TIMING 1;
#define DEFAULT_POLL_TIMING_NS 500000000;

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


#define RAIL_ATTACH_POINT "TRAFFIC_1_RAIL_ATTACH_POINT"

#define CENTRAL_ATTACH_POINT "TRAFFIC_1_DATA_SHARE_ATTACH_POINT"

#define TRAFFIC_OVERRIDE_ATTACH_POINT_NAME "TRAFFIC_1_OVERRIDE_ATTACH_POINT"

struct s_reply_message
{
	   struct _pulse hdr;  // Our real data comes after this header
       char data[3]; // Message we send back to clients to tell them the messages was processed correctly.
};

struct s_request_message
{
	   struct _pulse hdr;  // Our real data comes after this header
	   int ClientID;
       char data[3]; // Message we send back to clients to tell them the messages was processed correctly.
};


struct s_io_id
{
	uintptr_t gpio_LEDs;
	uintptr_t gpio_0_outputs;
	uintptr_t gpio_1_inputs;
	uintptr_t Switches_inputs;
};

struct s_traffic_data
{
	struct s_override_data override_data;
	struct s_traffic_message_data message_data;
	pthread_rwlock_t rwlock;
};

struct s_rail_data
{
	struct s_rail_message_data message_data;
	pthread_rwlock_t rwlock;
};

struct s_signal_msg
{
	struct _pulse pulse;
};

struct s_traffic_timing_event
{
	pthread_rwlock_t rwlock;
	struct sigevent traffic_timer_event;
	struct itimerspec instant_timer_spec;
	struct itimerspec green_timer_spec;
	struct itimerspec yellow_timer_spec;
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

struct s_traffic_control
{
	struct s_traffic_timing_event traffic_timing;
	struct s_rail_data rail_data;
	struct s_traffic_data traffic_data;
	struct s_io_id io_id;
};

void initialiseTrafficLightsData(struct s_traffic_data *tdata);
int initialiseTimers(struct s_traffic_timing_event *timing_data);
void *trafficLightControl(void* traffic_control_data);
void trafficOverrideHandler(struct s_traffic_data *tdata);
void initialiseRailData(struct s_rail_data *rdata);
void updateLights(struct s_traffic_control *tdata);
void* sensorPoll(void *data);
void printTrafficState(enum e_STATE state);
void resetTimer(struct s_traffic_timing_event *timer);


void printRailState(struct s_rail_data *data);


int initIO(struct s_traffic_control *data);
void unmapIO(struct s_traffic_control *data);


void* railServer(void *data);

void* dataShareServer(void *data);
void* overrideServer(void *data);

int main(void)
{

	// Puts it all in one nice neat struct for the thread stuff.
	struct s_traffic_control  traffic_control_data;


	/*
	 * These structure will be updated as sensor values are updated.
	 *	Initialises all of the traffic data to initial values.	 *
	 */
	//struct s_traffic_data traffic_data;
	initialiseTrafficLightsData(&traffic_control_data.traffic_data);
	puts("Traffic Light Data Initialised");

	//struct s_rail_data rail_data;
	initialiseRailData(&traffic_control_data.rail_data);
	puts("Rail Data Initialised");

	/*
	 * This holds all the timing event and signals for he traffic light.
	 * The following code initialises all of the timing and signal stuff.
	 */

	//struct s_traffic_timing_event traffic_timing;
	if(initialiseTimers(&traffic_control_data.traffic_timing) < 0)
	{
		puts("Error initialising timers");
		perror (NULL);
		exit (EXIT_FAILURE);
	}
	puts("Timers Initialised");

	if(!initIO(&traffic_control_data))
	{
		puts("Failed to initiate IO, exiting main...");
		return EXIT_SUCCESS;
	}


	// Creates thread and thread attributes for traffic lights
	pthread_t  traffic_control_thread;
	pthread_attr_t traffic_control_thread_attr;
	struct sched_param traffic_control_thread_param;
	pthread_attr_init(&traffic_control_thread_attr);
	traffic_control_thread_param.sched_priority = 10;
	pthread_attr_setschedparam(&traffic_control_thread_attr, &traffic_control_thread_param);

	// Creates thread and thread attributes for polling
	pthread_t  poll_thread;
	pthread_attr_t poll_thread_attr;
	struct sched_param poll_thread_param;
	pthread_attr_init(&poll_thread_attr);
	poll_thread_param.sched_priority = 5;
	pthread_attr_setschedparam(&poll_thread_attr, &poll_thread_param);

	// Creates thread attributes for rail message server
	pthread_t  rail_server_thread;
	pthread_attr_t rail_server_thread_attr;
	struct sched_param rail_server_thread_param;
	pthread_attr_init(&rail_server_thread_attr);
	rail_server_thread_param.sched_priority = 5;
	pthread_attr_setschedparam(&rail_server_thread_attr, &rail_server_thread_param);

	// Creates thread attributes for data share message server
	pthread_t  data_share_server_thread;
	pthread_attr_t data_share_server_thread_attr;
	struct sched_param data_share_server_thread_param;
	pthread_attr_init(&data_share_server_thread_attr);
	rail_server_thread_param.sched_priority = 5;
	pthread_attr_setschedparam(&data_share_server_thread_attr, &data_share_server_thread_param);

	// Creates thread and thread attributes for client
	pthread_t  override_thread;
	pthread_attr_t override_thread_attr;
	struct sched_param override_thread_param;
	pthread_attr_init(&override_thread_attr);
	override_thread_param.sched_priority = 10;
	pthread_attr_setschedparam(&override_thread_attr, &override_thread_param);


	// Starts traffic light control thread.
	pthread_create(&traffic_control_thread, &traffic_control_thread_attr, trafficLightControl, &traffic_control_data);
	// starts poll thread.
	pthread_create(&poll_thread, &poll_thread_attr, sensorPoll, &traffic_control_data);
	//start rail message thread
	pthread_create(&rail_server_thread, &rail_server_thread_attr, railServer, &traffic_control_data);
	//data share message thread
	pthread_create(&data_share_server_thread, &data_share_server_thread_attr, dataShareServer, &traffic_control_data);
	// Starts override thread
	pthread_create(&override_thread, &override_thread_attr, overrideServer, &traffic_control_data);


	void* ret;
	pthread_join(traffic_control_thread, &ret);

	return EXIT_SUCCESS;
}

void initialiseTrafficLightsData(struct s_traffic_data *tdata)
{
	struct s_traffic_data *traffic_data = (struct s_traffic_data*) tdata;
	traffic_data->message_data.automatic_mode = INACTIVE;
	traffic_data->override_data.automatic_mode = INACTIVE;
	traffic_data->override_data.override_active_flag = 0;
	traffic_data->message_data.east_west_green_length = 10;
	traffic_data->message_data.east_west_pedestrain_sensor = INACTIVE;
	traffic_data->message_data.north_south_green_length = 10;
	traffic_data->message_data.north_south_pedestrain_sensor = INACTIVE;
	traffic_data->message_data.current_state = EWR_NSR_INIT;
	traffic_data->message_data.previous_state = EWR_NSR_INIT;
	traffic_data->message_data.north_car_sensor = INACTIVE;
	traffic_data->message_data.south_car_sensor = INACTIVE;
	traffic_data->message_data.east_car_sensor = INACTIVE;
	traffic_data->message_data.west_car_sensor = INACTIVE;
}

void initialiseRailData(struct s_rail_data *rdata)
{
	struct s_rail_data *rail_data = (struct s_rail_data*) rdata;
	rail_data->message_data.east_north_train_sensor = INACTIVE;
	rail_data->message_data.east_south_train_sensor = INACTIVE;
	rail_data->message_data.west_north_train_sensor = INACTIVE;
	rail_data->message_data.west_south_train_sensor = INACTIVE;
	rail_data->message_data.current_state = GATE_CLOSED_INIT;
	rail_data->message_data.train_present_flag = NOT_PRESENT;
}

int initialiseTimers(struct s_traffic_timing_event *timing_data)
{
	struct s_traffic_timing_event *traffic_timing = (struct s_traffic_timing_event*) timing_data;
//	pthread_rwlock_rdlock(&traffic_timing->rwlock);
	// Creates communication channel
	traffic_timing->channel_id = ChannelCreate(0);
	traffic_timing->poll_channel_id = ChannelCreate(0);
	traffic_timing->traffic_timer_event.sigev_notify = SIGEV_PULSE;
	traffic_timing->poll_timer_event.sigev_notify = SIGEV_PULSE;
	// create a connection back to ourselves for the timer to send the pulse on
	traffic_timing->traffic_timer_event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, traffic_timing->channel_id, _NTO_SIDE_CHANNEL, 0);
	traffic_timing->poll_timer_event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, traffic_timing->poll_channel_id, _NTO_SIDE_CHANNEL, 0);
	if(traffic_timing->traffic_timer_event.sigev_coid == -1)
	{
	   printf("%s:  traffic timing couldn't ConnectAttach to self!\n", TRAFFIC_LIGHT_NAME);
	   return -1;
	}

	if(traffic_timing->poll_timer_event.sigev_coid == -1)
	{
	   printf("%s:  poll timing couldn't ConnectAttach to self!\n", TRAFFIC_LIGHT_NAME);
	   return -1;
	}
		// The following line might be needed?
	//event.sigev_priority = getprio(0);
	traffic_timing->traffic_timer_event.sigev_code = TRAFFIC_TIMER_PULSE_CODE;
	traffic_timing->poll_timer_event.sigev_code = POLL_TIMER_PULSE_CODE;
	//create the timer, binding it to the event
	if(timer_create(CLOCK_REALTIME, &traffic_timing->traffic_timer_event, &traffic_timing->timer_id) == -1)
	{
	   printf("%s:  couldn't create a traffic timer\n", TRAFFIC_LIGHT_NAME);
	   return -1;
	}

	if(timer_create(CLOCK_REALTIME, &traffic_timing->poll_timer_event, &traffic_timing->poll_timer_id) == -1)
	{
	   printf("%s:  couldn't create a poll timer\n", TRAFFIC_LIGHT_NAME);
	   return -1;
	}

	traffic_timing->green_timer_spec.it_value.tv_sec = DEFAULT_GREEN_TIMING;
	traffic_timing->green_timer_spec.it_value.tv_nsec = 0;
	traffic_timing->green_timer_spec.it_interval.tv_sec = DEFAULT_GREEN_TIMING;
	traffic_timing->green_timer_spec.it_interval.tv_nsec = 0;

	traffic_timing->yellow_timer_spec.it_value.tv_sec = DEFAULT_YELLOW_TIMING;
	traffic_timing->yellow_timer_spec.it_value.tv_nsec = 0;
	traffic_timing->yellow_timer_spec.it_interval.tv_sec = DEFAULT_YELLOW_TIMING;
	traffic_timing->yellow_timer_spec.it_interval.tv_nsec = 0;

	traffic_timing->instant_timer_spec.it_value.tv_sec = 0;
	traffic_timing->instant_timer_spec.it_value.tv_nsec = 1000;
	traffic_timing->instant_timer_spec.it_interval.tv_sec = 0;
	traffic_timing->instant_timer_spec.it_interval.tv_nsec = 1000;

	traffic_timing->poll_timer_spec.it_value.tv_sec = 0;
	traffic_timing->poll_timer_spec.it_value.tv_nsec = DEFAULT_POLL_TIMING_NS;
	traffic_timing->poll_timer_spec.it_interval.tv_sec = 0;
	traffic_timing->poll_timer_spec.it_interval.tv_nsec = DEFAULT_POLL_TIMING_NS;
	pthread_rwlock_unlock(&traffic_timing->rwlock);

	return 0;
}

void startTimer(timer_t timer, struct itimerspec spec)
{
	timer_settime(timer, 0, &spec, NULL);
}

void resetTimer(struct s_traffic_timing_event *timer)
{
	timer_delete(timer->timer_id);
	if(timer_create(CLOCK_REALTIME, &timer->traffic_timer_event, &timer->timer_id) == -1)
	{
	   printf("%s:  couldn't create a traffic timer\n", TRAFFIC_LIGHT_NAME);
	}
}
void *trafficLightControl(void *data)
{
	struct s_traffic_control *traffic_control_data = (struct s_traffic_control*) data;
	struct s_traffic_data *traffic_data = &traffic_control_data->traffic_data;
	struct s_rail_data *rail_data = &traffic_control_data->rail_data;
	struct s_traffic_timing_event *timing_data = &traffic_control_data->traffic_timing;
	// Do first timer.
	startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
	puts("trafficLightControl: first timer started");
	while(1)
	{

		// Wait for for message timing pulse
		timing_data->receive_id = MsgReceive(timing_data->channel_id, &timing_data->msg, sizeof(timing_data->msg), NULL);

		// Test if the message came from this process
		if(timing_data->receive_id == 0)
		{
			// Check we got a pulse
			if(timing_data->msg.pulse.code == TRAFFIC_TIMER_PULSE_CODE)
			{
				// All is good. We can try and calculate the traffic
				// lights now.
				pthread_rwlock_rdlock(&traffic_data->rwlock);
				pthread_rwlock_rdlock(&rail_data->rwlock);

				//pthread_rwlock_rdlock(&timing_data->rwlock);

				updateLights(traffic_control_data);
				/*
				 *	Handles a central control override situation.
				 */
				if(traffic_data->override_data.override_active_flag)
				{
					printf("\ntrafficLightControl: Central overide detected\n");
					trafficOverrideHandler(&traffic_control_data->traffic_data);
				}
				else
				{
					switch(traffic_data->message_data.current_state)
					{

						case EWR_NSR_INIT:
						{
							traffic_data->message_data.previous_state = EWR_NSR_INIT;
							traffic_data->message_data.current_state = EWR_NSR;
							//Sets timer to instantly call an output state update.
							startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							break;
						}
						case EWR_NSR:
						{
							/*
							 * 	Handles all the other situations
							 */
							switch(traffic_data->message_data.previous_state)
							{
								case EWR_NSR_INIT:
								{
									traffic_data->message_data.previous_state = EWR_NSR;
									traffic_data->message_data.current_state = EWG_NSR;
									break;
								}
								case EWY_NSR:
								{
									traffic_data->message_data.previous_state = EWR_NSR;
									traffic_data->message_data.current_state = EWR_NSG;
									break;
								}
								case EWR_NSY:
								{
									traffic_data->message_data.previous_state = EWR_NSR;
									traffic_data->message_data.current_state = EWG_NSR;
									break;
								}
								default:
								{
									traffic_data->message_data.previous_state = EWR_NSR;
									traffic_data->message_data.current_state = TRAFFIC_ERROR;
								}
							}
							//startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							sleep(1);
							break;
						}
						case EWG_NSR:
						{
							// Check if we have waited for the green time to finish.
							if(traffic_data->message_data.previous_state != EWG_NSR)
							{
								traffic_data->message_data.previous_state = EWG_NSR;
								startTimer(timing_data->timer_id, timing_data->green_timer_spec);
							}
							// If it has, we poll the sensors every second to see if we need to change the lights
							else if(	traffic_data->message_data.north_car_sensor == ACTIVE ||
								traffic_data->message_data.south_car_sensor == ACTIVE ||
								traffic_data->message_data.north_south_pedestrain_sensor == ACTIVE || traffic_data->message_data.automatic_mode == ACTIVE
							)
							{
								if(rail_data->message_data.train_present_flag == NOT_PRESENT)
								{
									traffic_data->message_data.previous_state = EWG_NSR;
									traffic_data->message_data.current_state = EWY_NSR;
									startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);

								}
								else
								{
									traffic_data->message_data.previous_state = EWG_NSR;
									startTimer(timing_data->timer_id, timing_data->green_timer_spec);
								}

							}
							else
							{
								traffic_data->message_data.previous_state = EWG_NSR;
								startTimer(timing_data->timer_id, timing_data->green_timer_spec);
							}
							break;
						}
						case EWY_NSR:
						{
							traffic_data->message_data.previous_state = EWY_NSR;
							traffic_data->message_data.current_state = EWR_NSR;
							startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							break;
						}
						case EWR_NSG:
						{
							if(rail_data->message_data.train_present_flag == PRESENT)
							{
								traffic_data->message_data.previous_state = EWR_NSG;
								traffic_data->message_data.current_state = EWR_NSY;
								startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							}
							// Check if we have waited for the green time to finish.
							else if(traffic_data->message_data.previous_state != EWR_NSG)
							{
								traffic_data->message_data.previous_state = EWR_NSG;
								startTimer(timing_data->timer_id, timing_data->green_timer_spec);
							}
							// If it has, we poll the sensors every second to see if we need to change the lights
							else if(	traffic_data->message_data.east_car_sensor == ACTIVE ||
								traffic_data->message_data.west_car_sensor == ACTIVE ||
								traffic_data->message_data.east_west_pedestrain_sensor == ACTIVE || traffic_data->message_data.automatic_mode == ACTIVE
							)
							{
								traffic_data->message_data.previous_state = EWR_NSG;
								traffic_data->message_data.current_state = EWR_NSY;
								startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							}
							else
							{
								traffic_data->message_data.previous_state = EWR_NSG;
								startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							}
							break;
						}
						case EWR_NSY:
						{
							traffic_data->message_data.previous_state = EWR_NSY;
							traffic_data->message_data.current_state = EWR_NSR;
							startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							break;
						}
						case TRAFFIC_ERROR:
						{
							startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							break;
						}
						default:
						{
							startTimer(timing_data->timer_id, timing_data->yellow_timer_spec);
							break;
						}
					}
				}
				pthread_rwlock_unlock(&traffic_data->rwlock);
				pthread_rwlock_unlock(&rail_data->rwlock);
				pthread_rwlock_unlock(&timing_data->rwlock);
			}
		}
	}
}

void trafficOverrideHandler(struct s_traffic_data *tdata)
{
	struct s_traffic_data *traffic_data = (struct s_traffic_data*) tdata;
	switch(traffic_data->override_data.override_new_state)
	{
		case EWR_NSR:
		{
			traffic_data->message_data.current_state = EWR_NSR;
			break;
		}
		case EWG_NSR:
		{
			traffic_data->message_data.current_state = EWG_NSR;
			break;
		}
		case EWY_NSR:
		{
			traffic_data->message_data.current_state = EWY_NSR;
			break;
		}
		case EWR_NSG:
		{
			traffic_data->message_data.current_state = EWR_NSG;
			break;
		}
		case EWR_NSY:
		{
			traffic_data->message_data.current_state = EWR_NSY;
			break;
		}
		case TRAFFIC_ERROR:
		{
			traffic_data->message_data.current_state = TRAFFIC_ERROR;
			break;
		}
		default:
		{
			traffic_data->message_data.current_state = TRAFFIC_ERROR;
			break;
		}
	}
}

void updateLights(struct s_traffic_control *tdata)
{
	struct s_traffic_control *traffic_control_data = (struct s_traffic_control*) tdata;
	struct s_traffic_data *traffic_data = &traffic_control_data->traffic_data;
	struct s_io_id *io_id = &traffic_control_data->io_id;

	volatile uint32_t	output = 0;

	switch(traffic_data->message_data.current_state)
	{
		case EWR_NSR_INIT:
		{
			output |= (1<<TRAFFIC_O_R_EW) | (1<<TRAFFIC_O_RED_N_S)| (1<<PED_O_R_EW) | (1<<PED_O_R_NS);
			break;
		}
		case EWR_NSR:
		{
			output |= (1<<TRAFFIC_O_R_EW) | (1<<TRAFFIC_O_RED_N_S) | (1<<PED_O_R_EW) | (1<<PED_O_R_NS);
			break;
		}
		case EWG_NSR:
		{
			output |= (1<<TRAFFIC_O_G_EW) | (1<<TRAFFIC_O_RED_N_S) | (1<<PED_O_R_EW);
			break;
		}
		case EWR_NSG:
		{
			output |= (1<<TRAFFIC_O_R_EW) | (1<<TRAFFIC_O_G_NS) | (1<<PED_O_R_NS);
			break;
		}
		case EWY_NSR:
		{
			output |= (1<<TRAFFIC_O_Y_EW) | (1<<TRAFFIC_O_RED_N_S) | (1<<PED_O_R_EW);
			break;
		}
		case EWR_NSY:
		{
			output |= (1<<TRAFFIC_O_R_EW) | (1<<TRAFFIC_O_Y_NS) | (1<<PED_O_R_NS);
			break;
		}
		case TRAFFIC_ERROR:
		{
			break;
		}
		default:
		{
			break;
		}
	}

	printTrafficState(traffic_data->message_data.current_state);
	puts("");
	//printf("output = %#04X\n\n",output);
	out32(io_id->gpio_0_outputs, output);

}

void* sensorPoll(void *data)
{
	struct s_traffic_control *traffic_control_data = (struct s_traffic_control*) data;
	struct s_traffic_data *traffic_data = &traffic_control_data->traffic_data;
	struct s_traffic_timing_event *timing_data = &traffic_control_data->traffic_timing;
	struct s_io_id *io_id = &traffic_control_data->io_id;
	volatile uint32_t	inputs = 0;
	volatile uint32_t	switches = 0;
	// start first timer.
	startTimer(timing_data->poll_timer_id, timing_data->poll_timer_spec);

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
				pthread_rwlock_rdlock(&traffic_data->rwlock);

				inputs  = in32(io_id->gpio_1_inputs);
			 	switches  = in32(io_id->Switches_inputs);
			 	//puts("polling sensors");
			 	//printf("inputs %d\n", inputs);
			 	//printf("switches %d\n", switches);


			 	if(((inputs >> TRAFFIC_I_N) & 1) || ((switches >> 0) & 1)){
			 		puts("north");
			 		traffic_data->message_data.north_car_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.north_car_sensor = 0;
			 	}

			 	if(((inputs >> TRAFFIC_I_S) & 1) || ((switches >> 1) & 1)){
			 		puts("south");
			 		traffic_data->message_data.south_car_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.south_car_sensor = 0;
			 	}

			 	if(((inputs >> TRAFFIC_I_E) & 1) || ((switches >> 2) & 1)){
			 		puts("east");
			 		traffic_data->message_data.east_car_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.east_car_sensor = 0;
			 	}

			 	if(((inputs >> TRAFFIC_I_W) & 1) || ((switches >> 3) & 1)){
			 		puts("west");
			 		traffic_data->message_data.west_car_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.west_car_sensor = 0;
			 	}

			 	if((inputs >> PED_I_NS) & 1){
			 		puts("NS_Pedestrian");
			 		traffic_data->message_data.north_south_pedestrain_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.north_south_pedestrain_sensor = 0;
			 	}

			 	if((inputs >> PED_I_EW) & 1){
			 		puts("EW_Pedestrian");
			 		traffic_data->message_data.east_west_pedestrain_sensor = 1;
			 	}
			 	else{
			 		traffic_data->message_data.east_west_pedestrain_sensor = 0;
			 	}

				pthread_rwlock_unlock(&traffic_data->rwlock);
				//sleep(1);
				startTimer(timing_data->poll_timer_id, timing_data->poll_timer_spec);
			}
		}
	}
}

void printTrafficState(enum e_STATE state)
{
	time_t time_of_day;
	time_of_day = time(NULL);
	printf("%s", ctime(&time_of_day));

	switch(state)
	{
		case EWR_NSR_INIT:
		{
			printf("EWR_NSR_INIT\n");
			break;
		}
		case EWR_NSR:
		{
			printf("EWR_NSR\n");
			break;
		}
		case EWG_NSR:
		{
			printf("EWG_NSR\n");
			break;
		}
		case EWR_NSG:
		{
			printf("EWR_NSG\n");
			break;
		}
		case EWY_NSR:
		{
			printf("EWY_NSR\n");
			break;
		}
		case EWR_NSY:
		{
			printf("EWR_NSY\n");
			break;
		}
		case TRAFFIC_ERROR:
		{
			printf("TRAFFIC_ERROR\n");
			break;
		}
	}
}

int initIO(struct s_traffic_control *data)
{

	struct s_traffic_control *traffic_control_data = (struct s_traffic_control*) data;
	struct s_io_id *io_id = &traffic_control_data->io_id;

	io_id->gpio_LEDs = mmap_device_io(PIO_SIZE, LEDs_BASE);
	if( !io_id->gpio_LEDs )
	{
		// An error has occurred
		perror("Can't map Control Base Module / gpio_LEDs");
		printf("Mapping IO ERROR! Main Terminated...!\n");
		return 0;
	}

	io_id->gpio_0_outputs = mmap_device_io(PIO_SIZE, GPIO_0_BASE);
	if( !io_id->gpio_0_outputs )
	{
		// An error has occurred
		perror("Can't map Control Base Module / gpio_0_outputs");
		printf("Mapping IO ERROR! Main Terminated...!\n");
		return 0;
	}

	io_id->gpio_1_inputs = mmap_device_io(PIO_SIZE, GPIO_1_BASE);
	if( !io_id->gpio_1_inputs )
	{
		// An error has occurred
		perror("Can't map Control Base Module / gpio_1_inputs");
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

void unmapIO(struct s_traffic_control *data)
{

	struct s_traffic_control *traffic_control_data = (struct s_traffic_control*) data;
	struct s_io_id *io_id = &traffic_control_data->io_id;

	munmap_device_io(io_id->gpio_LEDs, PIO_SIZE);
	munmap_device_io(io_id->gpio_0_outputs, PIO_SIZE);
	munmap_device_io(io_id->gpio_1_inputs, PIO_SIZE);
	munmap_device_io(io_id->Switches_inputs, PIO_SIZE_Switches);

}

void* railServer(void *data)
{

	puts("rail server thread started");

    name_attach_t *attach;
    struct s_traffic_control *traffic_control = (struct s_traffic_control*) data;
	struct s_rail_data *rail_data = (struct s_rail_data*) &traffic_control->rail_data;
	struct s_rail_message_data rail_message_local;

   // Create a local name (/dev/name/...)
   if ((attach = name_attach(NULL, RAIL_ATTACH_POINT, 0)) == NULL)
   {
       printf("\nFailed to name_attach on ATTACH_POINT: %s \n", RAIL_ATTACH_POINT);
       printf("\n Possibly another server with the same name is already running !\n");
       pthread_exit(EXIT_SUCCESS);
   }

   printf("Server Listening for Clients on ATTACH_POINT: %s \n", RAIL_ATTACH_POINT);

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
       rcvid = MsgReceive(attach->chid, &rail_message_local, sizeof(struct s_rail_message_data), NULL);

       if (rcvid == -1)  // Error condition, exit
       {
           printf("\nFailed to MsgReceive\n");
       }

       // did we receive a Pulse or message?
       // for Pulses:
       if (rcvid == 0)  //  Pulse received, work out what type
       {
           switch (rail_message_local.hdr.code)
           {
			   case _PULSE_CODE_DISCONNECT:
					// A client disconnected all its connections by running
					// name_close() for each name_open()  or terminated
				   if( Stay_alive == 0)
				   {
					   //ConnectDetach(msg.hdr.scoid);
					   printf("\nServer was told to Detach from ClientID:%d ...\n", rail_message_local.ClientID);
					   living = 0; // kill while loop
					   continue;
				   }
				   else
				   {
					   printf("\nServer received Detach pulse from ClientID:%d but rejected it ...\n", rail_message_local.ClientID);
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
		   if (rail_message_local.hdr.type == _IO_CONNECT)
		   {
			   MsgReply( rcvid, EOK, NULL, 0 );
			   printf("\n gns service is running....");
			   continue;	// go back to top of while loop
		   }
		   // Some other I/O message was received; reject it
		   if (rail_message_local.hdr.type > _IO_BASE && rail_message_local.hdr.type <= _IO_MAX )
		   {
			   MsgError( rcvid, ENOSYS );
			   printf("\n Server received and IO message and rejected it....");
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
		   pthread_rwlock_rdlock(&traffic_control->traffic_timing.rwlock);

			rail_data->message_data.ClientID = rail_message_local.ClientID;
			rail_data->message_data.hdr = rail_message_local.hdr;
			rail_data->message_data.lights_red_length = rail_message_local.lights_red_length;
			rail_data->message_data.east_north_train_sensor = rail_message_local.east_north_train_sensor;
			rail_data->message_data.train_present_flag = rail_message_local.train_present_flag;
			rail_data->message_data.current_state = rail_message_local.current_state;
			rail_data->message_data.previous_state = rail_message_local.previous_state;
			rail_data->message_data.east_north_train_sensor = rail_message_local.east_north_train_sensor;
			rail_data->message_data.east_south_train_sensor = rail_message_local.east_south_train_sensor;
			rail_data->message_data.west_north_train_sensor = rail_message_local.west_north_train_sensor;
			rail_data->message_data.west_south_train_sensor = rail_message_local.west_south_train_sensor;
			puts("Instant timer spec");
			//resetTimer(&traffic_control->traffic_timing);
			if(rail_data->message_data.train_present_flag == PRESENT)
			{
				startTimer(traffic_control->traffic_timing.timer_id, traffic_control->traffic_timing.yellow_timer_spec);
			}
			pthread_rwlock_unlock(&rail_data->rwlock);
			pthread_rwlock_unlock(&traffic_control->traffic_timing.rwlock);

		   printRailState(rail_data);
       }
       else
       {
		   printf("\nERROR: Server received something, but could not handle it correctly\n");
       }
   }
   puts("rail server thread ended");
   // Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
   name_detach(attach, 0);
   pthread_exit(EXIT_SUCCESS);
}



void printRailState(struct s_rail_data *data)
{
	struct s_rail_data *rail_data = (struct s_rail_data*) data;

	printf("east_north_train_sensor: %d \n", rail_data->message_data.east_north_train_sensor);
	printf("east_south_train_sensor: %d \n", rail_data->message_data.east_south_train_sensor);
	printf("west_north_train_sensor: %d \n", rail_data->message_data.west_north_train_sensor);
	printf("west_south_train_sensor: %d \n", rail_data->message_data.west_south_train_sensor);
}

void* dataShareServer(void *data)
{
	puts("data share server thread started");


    name_attach_t *attach;
    struct s_traffic_control *traffic_control = (struct s_traffic_control*) data;
	struct s_traffic_data *traffic_data = (struct s_traffic_data*) &traffic_control->traffic_data;
	struct s_traffic_message_data *traffic_message_reply =  (struct s_traffic_message_data*) &traffic_data->message_data;

	struct s_request_message request_message;

	traffic_message_reply->hdr.type = 0x01;
	traffic_message_reply->hdr.subtype = 0x00;


   // Create a local name (/dev/name/...)
   if ((attach = name_attach(NULL, CENTRAL_ATTACH_POINT, 0)) == NULL)
   {
       printf("\nFailed to name_attach on ATTACH_POINT: %s \n", CENTRAL_ATTACH_POINT);
       printf("\n Possibly another server with the same name is already running !\n");
       pthread_exit(EXIT_SUCCESS);
   }

   printf("Server Listening for Clients on ATTACH_POINT: %s \n", CENTRAL_ATTACH_POINT);

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
		   pthread_rwlock_rdlock(&traffic_data->rwlock);

		   if(MsgReply(rcvid, EOK, traffic_message_reply, sizeof(struct s_traffic_message_data))){
			   puts("message reply error");
		   //MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
		   }

		   pthread_rwlock_unlock(&traffic_data->rwlock);

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





void* overrideServer(void *data)
{

	puts("overrideServer thread started");

    name_attach_t *attach;
    struct s_traffic_control *traffic_control = (struct s_traffic_control*) data;
    struct s_traffic_data *traffic_data = (struct s_traffic_data*) &traffic_control->traffic_data;
    struct s_override_data *override_data = (struct s_override_data*) &traffic_data->override_data;

   // Create a local name (/dev/name/...)
   if ((attach = name_attach(NULL, TRAFFIC_OVERRIDE_ATTACH_POINT_NAME, 0)) == NULL)
   {
       printf("\nFailed to name_attach on TRAFFIC_OVERRIDE_ATTACH_POINT_NAME: %s \n", TRAFFIC_OVERRIDE_ATTACH_POINT_NAME);
       printf("\n Possibly another server with the same name is already running !\n");
       pthread_exit(EXIT_SUCCESS);
   }

   printf("overrideServer Listening for Clients on RAIL_OVERRIDE_ATTACH_POINT: %s \n", TRAFFIC_OVERRIDE_ATTACH_POINT_NAME);

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
		   pthread_rwlock_rdlock(&traffic_data->rwlock);
		   pthread_rwlock_rdlock(&traffic_control->traffic_timing.rwlock);

		   if(override_data->automatic_mode == ACTIVE || override_data->automatic_mode == INACTIVE)
		   {
			   traffic_data->message_data.automatic_mode =override_data->automatic_mode;
		   }
			if(override_data->override_active_flag)
			{
				trafficOverrideHandler(traffic_data);
				startTimer(traffic_control->traffic_timing.timer_id, traffic_control->traffic_timing.yellow_timer_spec);
			}
			else
			{
				if(override_data->override_new_state == EWR_NSR)
				{
					override_data->override_new_state = EWG_NSR;
				}
				trafficOverrideHandler(traffic_data);
				startTimer(traffic_control->traffic_timing.timer_id, traffic_control->traffic_timing.yellow_timer_spec);
			}


			pthread_rwlock_unlock(&traffic_data->rwlock);
			pthread_rwlock_unlock(&traffic_control->traffic_timing.rwlock);
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
