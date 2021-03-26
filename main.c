#include "main.h"

int main(int argc, char *argv[])
{
	SOCKET channel_s;
	struct sockaddr_in channel, sender, reciever, some_client;
	int slen, recv_len;
	char buf[BUFLEN];	
	slen = sizeof(sender);
	//////////////////////////////////////////////////////////////////////////
	// Line arguments handling
	//////////////////////////////////////////////////////////////////////////
	int channel_port = atoi(argv[1]);
	char* str_reciever_ip = argv[2];
	int reciever_port = atoi(argv[3]);
	int p_err = atoi(argv[4]);
	int seed = atoi(argv[5]);
	//////////////////////////////////////////////////////////////////////////
	// Socket handling
	//////////////////////////////////////////////////////////////////////////
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return EXIT_FAILURE;
	}
	//Create channel socket
	if ((channel_s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
		return EXIT_FAILURE;
	}
	printf("Socket created.\n");
	//Prepare the sockaddr_in structure & bind
	channel.sin_family = AF_INET;
	channel.sin_addr.s_addr = INADDR_ANY;
	channel.sin_port = htons(channel_port);	
	if (bind(channel_s, (struct sockaddr*)&channel, sizeof(channel)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		return EXIT_FAILURE;
	}
	printf("Bind done.\n");
	// Prepare the reciever sockaddr_in structure
	reciever.sin_family = AF_INET;
	reciever.sin_addr.s_addr = inet_addr(str_reciever_ip);
	reciever.sin_port = htons(reciever_port);
	//////////////////////////////////////////////////////////////////////////
	// Main channel loop
	// if got msg from sender --> add noise --> send to reciever
	// if got msg from reciever --> pass the msg as is to sender and close the socket
	//////////////////////////////////////////////////////////////////////////
	srand(seed);
	bool exit_loop = false;
	bool b_running_ok = true;
	while (!exit_loop)
	{		
		//memset(buf, '\0', BUFLEN);		
		//try to receive some data
		if ((recv_len = recvfrom(channel_s, buf, BUFLEN, 0, (struct sockaddr*)&some_client, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			b_running_ok = false;
			exit_loop = true;		
		}
		// if msg recieved from reciever --> send it to sender then close socket
		if ((some_client.sin_port == reciever.sin_port)&&(some_client.sin_addr.s_addr == reciever.sin_addr.s_addr)) {
			if (!b_sender_exist) {
				printf("sender not exist");
				return EXIT_FAILURE;
			}
			if (sendto(channel_s, buf, recv_len, 0, (struct sockaddr*)&sender, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				return EXIT_FAILURE;
			}
			fprintf(stderr, "sender:%s\n", inet_ntoa(sender.sin_addr));
			fprintf(stderr, "reciever:%s\n", inet_ntoa(reciever.sin_addr));
			fprintf(stderr, "%d bytes, flipped %d bits", total_bytes, total_flipped_bits);
			exit_loop = true;
		}
		else { // msg recieved from sender --> add noise and send to reciever
			b_sender_exist = true;
			memcpy(&sender, &some_client, sizeof(sender));
			total_bytes += recv_len;
			total_flipped_bits += add_random_noise(buf, recv_len, p_err);			
			if (sendall(channel_s, buf, &recv_len, reciever) == EXIT_FAILURE) {
				fprintf(stderr, "Erron in sendall()\n");
				b_running_ok = false;
				exit_loop = true;
			}
		}

	}
	closesocket(channel_s);
	WSACleanup();
	if (b_running_ok)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

int add_random_noise(char* buff, int data_len, int p_err) {
	int i, j, rand_res, num_of_flip_bits = 0;
	for (i = 0; i < data_len; i++) { //loop over all bytes in buff
		for (j = 1; j <= MSB_BIT_WEIGHT; j = j * 2) { //loop over all bits in byte and flip if need
			rand_res = (rand() % 2) * 65536 + rand(); // generate random number from 0 to 65536
			if (rand_res <= p_err) { //to check if start in 0 or 1
				num_of_flip_bits++;
				buff[i] ^= j;
			}				
		}
	}
	return num_of_flip_bits;
}

int sendall(int s, char* buf, int* len, struct sockaddr_in client_addr) {
	int total = 0; /* how many bytes we've sent */
	int bytesleft = *len; /* how many we have left to send */
	int n;
	int slen;
	slen = sizeof(struct sockaddr_in*);

	while (total < *len) {
		n = sendto(s, buf + total, bytesleft, 0, (struct sockaddr*)&client_addr, sizeof(struct sockaddr_in));
		if (n == SOCKET_ERROR)
			return EXIT_FAILURE;
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	*len = total; /* return number actually sent here */
	return n == -1 ? EXIT_FAILURE : EXIT_SUCCESS; /*-1 on failure, 0 on success */
}