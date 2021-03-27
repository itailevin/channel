//
// Created by galpo on 25/03/2021.
//

#ifndef COMPUTERNETWORKS_FUNCTIONS_H
#define COMPUTERNETWORKS_FUNCTIONS_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define CHUNK_SENDER 11
#define CHUNK_RECEIVER 15
#define MAX_CHUNK 15
#define BEFORE_HAMMING_SIZE 11
#define AFTER_HAMMING_SIZE 15
#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdio.h"
#include "winsock2.h"
#include <stdbool.h>
#include <windows.h>
#define BUFLEN 512	//Max length of buffer
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#pragma warning(disable:4996)

typedef struct {
    char* str;
    int length;
} String;

int is_value_in_array(int val, const int* arr, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}

String string_to_binary(String string) {

    char* binary = (char*)malloc(string.length * 8 + 1);
    binary[string.length * 8] = '\0';

    if (string.length == 0) {
        binary[0] = '\0';
        String s = { binary, 1 };
        return s;
    }
    // loop over the input-characters
    for (int i = 0; i < string.length; i++) {
        /* perform bitwise AND for every bit of the character */
        // loop over the input-character bits
        for (int j = 0; j < 8; j++) {
            binary[i * 8 + j] = (string.str[i] & 1 << (7 - j)) ? '1' : '0';
        }
    }
    String s = { binary, string.length * 8 };
    return s;
}

String bits_to_string(String bits) {
    char* string = (char*)malloc(bits.length / 8 + 1);
    string[bits.length / 8] = '\0';

    if (bits.length == 0) {
        string[0] = '\0';
        String s = { string, 1 };
        return s;
    }
    for (int i = 0; i < bits.length; i += 8) {
        int character = 0;
        for (int j = 0; j < 8; j++) {
            character += (bits.str[i + 7 - j] - '0') * pow(2, j);
        }
        string[i / 8] = character;
    }
    String s = { string, bits.length / 8 };
    return s;
}

String read_file_in_chunks(const char* file_name, bool isSender) {
    static size_t bytes_read_sender = 0;
    static size_t bytes_read_receiver = 0;
    size_t CHUNK = isSender ? CHUNK_SENDER : CHUNK_RECEIVER;
    FILE* file = fopen(file_name, "rb");
    if (!file) {
        printf("Cannot open '%s': %s\n", file_name, strerror(errno));
        exit(1);
    }
    size_t bytes_read = isSender ? bytes_read_sender : bytes_read_receiver;
    fseek(file, bytes_read, SEEK_SET);
    size_t nread;
    size_t length;
    char* buf = (char*)malloc(MAX_CHUNK + 1);
    if ((nread = fread(buf, 1, CHUNK, file)) > 0) {
        buf[nread] = '\0';
        length = nread;
        bytes_read = nread;
    }
    else {
        length = 0;
    }
    if (ferror(file)) {
        fprintf(stderr, "file error\n");
        free(buf);
        exit(1);
    }
    fclose(file);
    if (isSender) {
        bytes_read_sender += bytes_read;
    }
    else {
        bytes_read_receiver += bytes_read;
    }
    String string = { buf, length };
    return string;
}

void append_to_file(char* file_name, String string) {
    FILE* file = fopen(file_name, "a+b");
    if (!file) {
        printf("Cannot open '%s': %s\n", file_name, strerror(errno));
        exit(1);
    }
    fwrite(string.str, 1, string.length, file);
    if (ferror(file)) {
        fprintf(stderr, "file error\n");
        exit(1);
    }
    fclose(file);
}

String drop_first_bit(String bits, int block_size) {
    char* string = (char*)malloc(bits.length / block_size * (block_size - 1) + 1);
    int bits_copied = 0;
    for (int i = 0; i < bits.length; i += block_size) {
        for (int j = 1; j < block_size; j++) {
            string[bits_copied++] = bits.str[i + j];
        }
    }
    string[bits_copied] = '\0';
    String s = { string, bits_copied };
    return s;
};

String add_first_bit(String bits, int block_size) {
    char* string = (char*)malloc(bits.length / block_size * (block_size + 1) + 1);
    int bits_copied = 0;
    for (int i = 0; i < bits.length; i += block_size + 1) {
        string[i] = '0';
        for (int j = 0; j < block_size; j++) {
            string[i + j + 1] = bits.str[bits_copied++];
        }
    }
    string[bits.length / block_size * (block_size + 1)] = '\0';
    String s = { string, bits.length / block_size * (block_size + 1) };
    return s;
};

String hamming_wrap(String bits) {
    char* hamming_wrapped = (char*)malloc(bits.length / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1) + 1);
    hamming_wrapped[bits.length / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1)] = '\0';
    int parity_bits[AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1];
    parity_bits[0] = 0;
    for (int i = 1; i < AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1; i++) {
        parity_bits[i] = (int)(pow(2, i - 1));
    }
    for (int block = 0; block < bits.length; block += BEFORE_HAMMING_SIZE) {
        int parity_all = 0;
        for (int i = 0; i < BEFORE_HAMMING_SIZE; i++) {
            parity_all ^= bits.str[block + i] - '0';
        }
        hamming_wrapped[block / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1)] = (char)(parity_all + '0');
        int bits_copied = 0;
        for (int i = 0; i < (AFTER_HAMMING_SIZE + 1); i++) {
            if (is_value_in_array(i, parity_bits, AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1)) {
                continue;
            }
            hamming_wrapped[block / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1) + i] = bits.str[block + bits_copied++];
        }
        for (int i = 1; i < AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1; i++) {
            int parity = 0;
            for (int j = 0; j < (AFTER_HAMMING_SIZE + 1); j++) {
                if ((j & parity_bits[i]) == parity_bits[i] && j != parity_bits[i]) {
                    parity ^= hamming_wrapped[block / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1) + j] - '0';
                }
            }
            hamming_wrapped[block / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1) + parity_bits[i]] = (char)(parity + '0');
        }
    }
    String s = { hamming_wrapped ,bits.length / BEFORE_HAMMING_SIZE * (AFTER_HAMMING_SIZE + 1) };
    return s;
}

String unwrap_hamming(String hamming_wrapped) {
    char* string = (char*)malloc(hamming_wrapped.length / (AFTER_HAMMING_SIZE + 1) * BEFORE_HAMMING_SIZE + 1);
    string[hamming_wrapped.length / (AFTER_HAMMING_SIZE + 1) * BEFORE_HAMMING_SIZE] = '\0';
    int parity_bits[AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1];
    parity_bits[0] = 0;
    for (int i = 1; i < AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1; i++) {
        parity_bits[i] = (int)(pow(2, i - 1));
    }
    for (int block = 0; block < hamming_wrapped.length; block += (AFTER_HAMMING_SIZE + 1)) {
        int mistake_location = 0;
        for (int i = 1; i < AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1; i++) {
            int parity = 0;
            for (int j = 0; j < (AFTER_HAMMING_SIZE + 1); j++) {
                if ((j & parity_bits[i]) == parity_bits[i] && j != parity_bits[i]) {
                    parity ^= hamming_wrapped.str[block + j] - '0';
                }
            }
            if (parity != hamming_wrapped.str[block + parity_bits[i]] - '0') {
                printf("mistake block #%d, parity position %d\n", block / (AFTER_HAMMING_SIZE + 1), parity_bits[i]);
                mistake_location += parity_bits[i];
            }
        }
        if (mistake_location > 0) {
            hamming_wrapped.str[block + mistake_location] = ((hamming_wrapped.str[block + mistake_location] - '0') ^ 1) + '0';
        }
        int bits_copies = 0;
        for (int i = 0; i < (AFTER_HAMMING_SIZE + 1); i++) {
            if (is_value_in_array(i, parity_bits, AFTER_HAMMING_SIZE - BEFORE_HAMMING_SIZE + 1)) {
                continue;
            }
            string[block / (AFTER_HAMMING_SIZE + 1) * BEFORE_HAMMING_SIZE + bits_copies++] = hamming_wrapped.str[block + i];
        }
    }
    String s = { string ,hamming_wrapped.length / (AFTER_HAMMING_SIZE + 1) * BEFORE_HAMMING_SIZE };
    return s;
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

#endif //COMPUTERNETWORKS_FUNCTIONS_H
