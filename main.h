#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdio.h"
#include "winsock2.h"
#include <stdbool.h>
#define MSB_BIT_WEIGHT 128
#define BUFLEN 512	//Max length of buffer
#pragma comment(lib,"ws2_32.lib") //Winsock Library

int total_bits = 0;
int number_of_total_flipped_bits = 0;
int b_sender_exist = false;
int add_noise(char* buff, int data_len, int p_err);






