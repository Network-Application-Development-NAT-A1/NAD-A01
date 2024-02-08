/*
	Reliability and Flow Control Example
	From "Networking for Game Programmers" - http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <gaffer@gaffer.org>
*/

#pragma warning(disable : 4996)
#pragma warning(suppress : 4996)
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Net.h"

//#define SHOW_ACKS

using namespace std;
using namespace net;

const int ServerPort = 30000;
const int ClientPort = 30001;
const int ProtocolId = 0x11223344;
const float DeltaTime = 1.0f / 30.0f;
const float SendRate = 1.0f / 30.0f;
const float TimeOut = 10.0f;
const int PacketSize = 256;//Edit Please::::: Assume a constant 900 to transfer the MetaData file structure.




class FlowControl
{
public:

	FlowControl()
	{
		printf("flow control initialized\n");
		Reset();
	}

	void Reset()
	{
		mode = Bad;
		penalty_time = 4.0f;
		good_conditions_time = 0.0f;
		penalty_reduction_accumulator = 0.0f;
	}

	void Update(float deltaTime, float rtt)
	{
		const float RTT_Threshold = 250.0f;

		if (mode == Good)
		{
			if (rtt > RTT_Threshold)
			{
				printf("*** dropping to bad mode ***\n");
				mode = Bad;
				if (good_conditions_time < 10.0f && penalty_time < 60.0f)
				{
					penalty_time *= 2.0f;
					if (penalty_time > 60.0f)
						penalty_time = 60.0f;
					printf("penalty time increased to %.1f\n", penalty_time);
				}
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				return;
			}

			good_conditions_time += deltaTime;
			penalty_reduction_accumulator += deltaTime;

			if (penalty_reduction_accumulator > 10.0f && penalty_time > 1.0f)
			{
				penalty_time /= 2.0f;
				if (penalty_time < 1.0f)
					penalty_time = 1.0f;
				printf("penalty time reduced to %.1f\n", penalty_time);
				penalty_reduction_accumulator = 0.0f;
			}
		}

		if (mode == Bad)
		{
			if (rtt <= RTT_Threshold)
				good_conditions_time += deltaTime;
			else
				good_conditions_time = 0.0f;

			if (good_conditions_time > penalty_time)
			{
				printf("*** upgrading to good mode ***\n");
				good_conditions_time = 0.0f;
				penalty_reduction_accumulator = 0.0f;
				mode = Good;
				return;
			}
		}
	}

	float GetSendRate()
	{
		return mode == Good ? 30.0f : 10.0f;
	}

private:

	enum Mode
	{
		Good,
		Bad
	};

	Mode mode;
	float penalty_time;
	float good_conditions_time;
	float penalty_reduction_accumulator;
};

// ----------------------------------------------

int main(int argc, char* argv[])
{
	// parse command line

	enum Mode
	{
		Client,
		Server
	};

	Mode mode = Server;
	Address address;
	FileMetaData metaData;			//Client 1: Initialize of Struct of Metadata

	if (argc >= 5)
	{
		//TO DO 1: Add additional argument handling logic
		int a, b, c, d;
		if (sscanf(argv[1], "%d.%d.%d.%d", &a, &b, &c, &d))//Client 1: Get 4 Parameters from user: Ip address, File name, file size, file format
		{
			//Client1: Ip Address of 4 Parameters 
			mode = Client;
			address = Address(a, b, c, d, ServerPort);
			
			//Client 1:Enter filename, filesize, fileformat as user's parameters.
			strncpy(metaData.fileName, argv[2], FILENAME_MAX);   
			metaData.fileSize = atoi(argv[3]);                  
			strncpy(metaData.fileFormat, argv[4], FORMAT_MAX);  
		}
	}

	// initialize

	if (!InitializeSockets())
	{
		printf("failed to initialize sockets\n");
		return 1;
	}

	ReliableConnection connection(ProtocolId, TimeOut);

	const int port = mode == Server ? ServerPort : ClientPort;

	if (!connection.Start(port))
	{
		printf("could not start connection on port %d\n", port);
		return 1;
	}

	if (mode == Client)
		connection.Connect(address);
	else
		connection.Listen();

	bool connected = false;
	float sendAccumulator = 0.0f;
	float statsAccumulator = 0.0f;

	FlowControl flowControl;

	while (true)
	{
		// update flow control

		if (connection.IsConnected())
			flowControl.Update(DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f);

		const float sendRate = flowControl.GetSendRate();

		// detect changes in connection state

		if (mode == Server && connected && !connection.IsConnected())
		{
			flowControl.Reset();
			printf("reset flow control\n");
			connected = false;
		}

		if (!connected && connection.IsConnected())
		{
			printf("client connected to server\n");
			connected = true;
		}

		if (!connected && connection.ConnectFailed())
		{
			printf("connection failed\n");
			break;
		}


		//Client1: Edit Please:::::Verifying that the server has received the correct input of filename, filesize, fileformat. Server-side correction request.
		//							1. Instead of str[] (Hello World), print the contents of fileName, fileSize, and fileFormat, which are the contents of the metaData structure sent by the client to the server.
		// 
		// 

		// send and receive packets
		sendAccumulator += DeltaTime;
		unsigned char str[] = "Hello World ";
		size_t strSize = sizeof(str) - 1;
		unsigned int count = 0;

		while (sendAccumulator > 1.0f / sendRate)
		{
			unsigned char packet[PacketSize];
			memset(packet, 0, sizeof(packet));

			// Edit Please::::::Copy the metaData to the packet.
			size_t metaDataSize = sizeof(FileMetaData);
			if (metaDataSize <= PacketSize)
			{
				memcpy(packet, &metaData, metaDataSize);
			}

			// [client action]
			// TO DO 1: Need to implement logic to read files from disk.
			// TO DO 2: Add file metadata to the packet here.
			// TO DO 3: Break the file into parts and send each part here.
			connection.SendPacket(packet, sizeof(packet));
			sendAccumulator -= 1.0f / sendRate;
			count++;
		}

		//// Packet receiving logic on the server side
		while (true)
		{
			unsigned char packet[PacketSize];//Client1: Edit the packet Size
			int bytes_read = connection.ReceivePacket(packet, sizeof(packet));

			if (bytes_read == 0)
				break;

			// [server action]
			// TO DO 1: Extract file metadata from received packets.
			// TO DO 2: Process the received file part here and save it to disk.
			// TO DO 3: Validate that the entire file was received and verify file integrity here.

			// Edit Please::::: Client 1 : Extracting metadata from packets
			FileMetaData* receivedMetaData = reinterpret_cast<FileMetaData*>(packet);
			// Edit Please:::: Client1: Extracted metadata output
			printf("Received fileName: %s\n", receivedMetaData->fileName);
			printf("Received fileSize: %d\n", receivedMetaData->fileSize);
			printf("Received fileFormat: %s\n", receivedMetaData->fileFormat);
		}

		// show packets that were acked this frame

#ifdef SHOW_ACKS
		unsigned int* acks = NULL;
		int ack_count = 0;
		connection.GetReliabilitySystem().GetAcks(&acks, ack_count);
		if (ack_count > 0)
		{
			printf("acks: %d", acks[0]);
			for (int i = 1; i < ack_count; ++i)
				printf(",%d", acks[i]);
			printf("\n");
		}
#endif

		// update connection

		connection.Update(DeltaTime);

		// show connection stats

		statsAccumulator += DeltaTime;

		while (statsAccumulator >= 0.25f && connection.IsConnected())
		{
			float rtt = connection.GetReliabilitySystem().GetRoundTripTime();

			unsigned int sent_packets = connection.GetReliabilitySystem().GetSentPackets();
			unsigned int acked_packets = connection.GetReliabilitySystem().GetAckedPackets();
			unsigned int lost_packets = connection.GetReliabilitySystem().GetLostPackets();

			float sent_bandwidth = connection.GetReliabilitySystem().GetSentBandwidth();
			float acked_bandwidth = connection.GetReliabilitySystem().GetAckedBandwidth();

			printf("rtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps\n",
				rtt * 1000.0f, sent_packets, acked_packets, lost_packets,
				sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f,
				sent_bandwidth, acked_bandwidth);

			statsAccumulator -= 0.25f;
		}

		net::wait(DeltaTime);
	}

	ShutdownSockets();

	return 0;
}
