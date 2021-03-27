//
// Created by galpo on 23/03/2021.
//

#include "functions.h"

int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("bad input");
        exit(1);
    }

    SOCKET sender_s;
    struct sockaddr_in channel;
    int slen;
    slen = sizeof(channel);
    char buf[BUFLEN];
    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* file_name = argv[3];
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return EXIT_FAILURE;
    }
    //Create channel socket
    if ((sender_s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Socket created.\n");
    //Prepare the sockaddr_in structure & bind
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = inet_addr(ip);
    channel.sin_port = htons(port);
    //////////////////////////////////////////////////////////////////////////
    bool exit_loop = false;

    String string;
    while ((string = read_file_in_chunks(file_name, true)).length > 0) {
        printf("%s\n", string.str);
        String str_in_bits = string_to_binary(string);
        printf("%s\n", str_in_bits.str);
        String hamming_wrapped = hamming_wrap(str_in_bits);
        printf("%s\n", hamming_wrapped.str);
        String final_wrapped = drop_first_bit(hamming_wrapped, AFTER_HAMMING_SIZE + 1);
        printf("%s\n", final_wrapped.str);
        String bits_to_str = bits_to_string(final_wrapped);
        printf("%s\n", bits_to_str.str);
        if (sendall(sender_s, bits_to_str.str, &bits_to_str.length, channel) == EXIT_FAILURE) {
            fprintf(stderr, "Erron in sendall()\n");
        }
        free(string.str);
        free(str_in_bits.str);
        free(hamming_wrapped.str);
        free(final_wrapped.str);
        free(bits_to_str.str);
    }
    if (recvfrom(sender_s, buf, BUFLEN, 0, (struct sockaddr*)&channel, &slen) == SOCKET_ERROR) {
        printf("recvfrom() failed with error code : %d", WSAGetLastError());
    }

    closesocket(sender_s);
    WSACleanup();
    return 0;
}