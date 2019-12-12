/**
 * @file	task_communicate.c
 * @author  Eriks Zaharans
 *   Modified by Sylvain Lapeyrade & Reda Bourakkadi
 * 
 * @date    31 Oct 2013
 *   Last Update: 12/12/2019
 *
 * @section DESCRIPTION
 * 
 * TDDD07 Real Time Systems: Lab 2
 *
 * Communicate Task.
 */

/* -- Includes -- */
/* system libraries */

/* project libraries */
#include "task.h"

/**
 * Communication (receive and send data)
 */
void task_communicate(void)
{
	// Check if task is enabled
	if (g_task_communicate.enabled == s_TRUE)
	{
		// Loacal variables
		void *data;	// Void pointer for data
		int data_type; // Data type

		// UDP Packet
		char udp_packet[g_config.udp_packet_size];
		int udp_packet_len;

		// Protocol
		protocol_t packet;

		//Start the new sequence
		int seq = 0; // Massi thing
		//In principle I want to send all the data in the buffer
		int last_id = g_list_send->count; // Massi thing

		// Respectively the list for pheromone, stream data and
		// a pointer to a doublylinkedlist_t for designating list
		doublylinkedlist_t *pheromone_list = doublylinkedlist_init();
		doublylinkedlist_t *stream_list = doublylinkedlist_init();
		doublylinkedlist_t **reference_list;

		// --------------------------------------------------
		//	LAB 2 starts here
		// --------------------------------------------------
		printf("i'm sending stuff : %d\n", sizeof(g_list_send));

		// Data rate is 153600 bits per second so we have:
		// 153600/8 = 19200 bytes and since there are 8 robots:
		// 19200/8 = 2400. We have taken 2160 to have 10% safety
		float max_data_to_send = 2160;
		float critical_data_sent = 0;
		float pheromone_data_sent = 0;
		float stream_data_sent = 0;

		float data_sent = 0; // Counter of bytes sent

		/* --- Send Data --- */

		printf("Sending Robot position and Victim position.\n");
		// While the maximum data sendable isn't reached
		while (g_list_send->count != 0)
		{
			seq++;

			// Allocate memory for data structure
			switch (g_list_send->first->data_type)
			{
			// Robot pose
			case s_DATA_STRUCT_TYPE_ROBOT:
				data = (void *)malloc(sizeof(robot_t));
				break;
			// Victim information
			case s_DATA_STRUCT_TYPE_VICTIM:
				data = (void *)malloc(sizeof(victim_t));
				break;
			// Pheromone map
			case s_DATA_STRUCT_TYPE_PHEROMONE:
				data = (void *)malloc(sizeof(pheromone_map_sector_t));
				reference_list = &pheromone_list; // References pheromone list
				break;
			// Command (for future use)
			case s_DATA_STRUCT_TYPE_CMD:
				data = (void *)malloc(sizeof(command_t));
				break;
			case s_DATA_STRUCT_TYPE_STREAM:
				data = (void *)malloc(sizeof(stream_t));
				reference_list = &stream_list; // References stream list
				break;
			// Other
			default:
				// Do nothing
				continue;
				break;
			}

			// Get data from the list
			doublylinkedlist_remove(g_list_send, g_list_send->first, data, &data_type);
			if (data_sent < max_data_to_send) // If there is still space for sending
			{
				// If the data is critical
				if (data_type == s_DATA_STRUCT_TYPE_ROBOT || data_type == s_DATA_STRUCT_TYPE_VICTIM)
				{
					// Encode data into UDP packet
					protocol_encode(udp_packet,
									&udp_packet_len,
									s_PROTOCOL_ADDR_BROADCAST,
									g_config.robot_id,
									g_config.robot_team,
									s_PROTOCOL_TYPE_DATA,
									seq,
									g_message_sequence_id,
									last_id,
									data_type,
									data);
					// Broadcast packet
					data_sent += udp_packet_len;
					udp_broadcast(g_udps, udp_packet, udp_packet_len);
				}
				else if (data_type == s_DATA_STRUCT_TYPE_PHEROMONE)
				{ // Insert in pheromone list
					doublylinkedlist_insert_beginning(*reference_list, data, data_type);
				}
				else if (data_type == s_DATA_STRUCT_TYPE_STREAM)
				{ // Insert in stream list
					doublylinkedlist_insert_beginning(*reference_list, data, data_type);
				}
			}
			// Free memory
			free(data);
		}

		critical_data_sent = data_sent / max_data_to_send;
		printf("Critical data sent %f\n", critical_data_sent);

		// If there is still pheromone data and still space to send
		while (pheromone_list->count > 0 && data_sent <= max_data_to_send)
		{

			data = (void *)malloc(sizeof(pheromone_map_sector_t));
			doublylinkedlist_remove(pheromone_list, pheromone_list->first, data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
							&udp_packet_len,
							s_PROTOCOL_ADDR_BROADCAST,
							g_config.robot_id,
							g_config.robot_team,
							s_PROTOCOL_TYPE_DATA,
							seq,
							g_message_sequence_id,
							last_id,
							data_type,
							data);

			data_sent += udp_packet_len;
			if (data_sent <= max_data_to_send) // If there is still space for sending
			{
				// Broadcast packet
				udp_broadcast(g_udps, udp_packet, udp_packet_len);
			}
			else
			{
				data_sent -= udp_packet_len;
			}
			// Free memory
			free(data);
		}
		doublylinkedlist_destroy(pheromone_list); // Free the pheromone list

		pheromone_data_sent = data_sent / max_data_to_send - critical_data_sent;
		printf("Pheromone data sent %f\n", pheromone_data_sent);

		// If there is still stream data and still space to send
		while (stream_list->count > 0 && data_sent <= max_data_to_send)
		{
			data = (void *)malloc(sizeof(stream_t));
			doublylinkedlist_remove(stream_list, stream_list->first, data, &data_type);

			// Encode data into UDP packet
			protocol_encode(udp_packet,
							&udp_packet_len,
							s_PROTOCOL_ADDR_BROADCAST,
							g_config.robot_id,
							g_config.robot_team,
							s_PROTOCOL_TYPE_DATA,
							seq,
							g_message_sequence_id,
							last_id,
							data_type,
							data);

			data_sent += udp_packet_len;
			if (data_sent <= max_data_to_send) // If there is still space for sending
			{
				// Broadcast packet
				udp_broadcast(g_udps, udp_packet, udp_packet_len);
			}
			else
			{
				data_sent -= udp_packet_len;
			}

			// Free memory
			free(data);
		}
		doublylinkedlist_destroy(stream_list); // Free the stream list

		stream_data_sent = data_sent / max_data_to_send - pheromone_data_sent - critical_data_sent;
		printf("Stream data sent %f\n", stream_data_sent);

		/* --- Receive Data --- */
		// Receive packets, decode and forward to proper process
		/* printf("paquets recieved = %d \n", udp_receive(g_udps, udp_packet, &udp_packet_len)); */
		// Receive UDP packet
		while (udp_receive(g_udps, udp_packet, &udp_packet_len) == s_OK)
		{
			// Decode packet
			//printf("%s\n",udp_packet);
			if (protocol_decode(&packet, udp_packet, udp_packet_len, g_config.robot_id, g_config.robot_team) == s_OK)
			{
				// Now decoding depends on the type of the packet
				switch (packet.type)
				{
				// ACK
				case s_PROTOCOL_TYPE_ACK:
					// Do nothing
					break;

				//Massi: go_ahead packet
				case s_PROTOCOL_TYPE_GO_AHEAD:
				{
					// Declare go ahead command
					command_t go_ahead;
					go_ahead.cmd = s_CMD_GO_AHEAD;
					// Redirect to mission by adding it to the queue
					queue_enqueue(g_queue_mission, &go_ahead, s_DATA_STRUCT_TYPE_CMD);

					// Debuging stuff
					debug_printf("GO_AHEAD RECEIVED for robot %d team %d\n", packet.recv_id, packet.send_team);
					// Calculate time from packet (ms and s)
					int send_time_s = floor(packet.send_time / 1000);
					int send_time_ms = packet.send_time % 1000;
					int now = floor(((long long)timelib_unix_timestamp() % 60000) / 1000);
					debug_printf("GO_AHEAD_TIME: %d (%d)\n", send_time_s, now);

					break;
				}
				// Data
				case s_PROTOCOL_TYPE_DATA:
					// Continue depending on the data type
					switch (packet.data_type)
					{
					// Robot pose
					case s_DATA_STRUCT_TYPE_ROBOT:
						debug_printf("received robot\n");
						// Do nothing
						break;
					// Victim information
					case s_DATA_STRUCT_TYPE_VICTIM:
						debug_printf("received victim\n");
						// Redirect to mission by adding it to the queue
						queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_VICTIM);
						break;
					// Pheromone map
					case s_DATA_STRUCT_TYPE_PHEROMONE:
						debug_printf("received pheromone\n");
						// Redirect to navigate by adding it to the queue
						queue_enqueue(g_queue_navigate, packet.data, s_DATA_STRUCT_TYPE_PHEROMONE);

						break;
					// Command
					case s_DATA_STRUCT_TYPE_CMD:
						debug_printf("received CMD\n");
						// Redirect to mission by adding it to the queue
						queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_CMD);
						break;
					case s_DATA_STRUCT_TYPE_STREAM:
						debug_printf("received data stream item\n");
						break;
					// Other
					default:
						// Do nothing
						break;
					}
				// Other ?
				default:
					// Do nothing
					break;
				}

				// Free memory (only if data packet was received!)
				if (packet.type == s_PROTOCOL_TYPE_DATA)
					free(packet.data);
			}
		}

		// Increase msg sequance id
		g_message_sequence_id++;
	}
}