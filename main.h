#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdio.h"
#include "winsock2.h"
#include <stdbool.h>
#define MSB_BIT_WEIGHT 128
#define BUFLEN 512	//Max length of buffer
#pragma comment(lib,"ws2_32.lib") //Winsock Library

int total_bytes = 0;
int total_flipped_bits = 0;
int b_sender_exist = false;
int add_random_noise(char* buff, int data_len, int p_err);
int sendall(int s, char* buf, int* len, struct sockaddr_in client_addr);






