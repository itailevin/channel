//
// Created by galpo on 25/03/2021.
//

#include "functions.h"

int main(int argc, char* argv[]) {

    if (argc != 3) {
        printf("bad input");
        exit(1);
    }

    SOCKET receiver_s;
    struct sockaddr_in receiver, channel;
    int slen, recv_len;
    char str[MAX_CHUNK];
    slen = sizeof(receiver);
    int port = atoi(argv[1]);
    char* file_name = argv[2];
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return EXIT_FAILURE;
    }
    //Create channel socket
    if ((receiver_s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Socket created.\n");
    //Prepare the sockaddr_in structure & bind
    receiver.sin_family = AF_INET;
    receiver.sin_addr.s_addr = INADDR_ANY;
    receiver.sin_port = htons(port);
    if (bind(receiver_s, (struct sockaddr*)&receiver, sizeof(receiver)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Bind done.\n");
    //////////////////////////////////////////////////////////////////////////
    // Main channel loop
    // if got msg from sender --> add noise --> send to reciever
    // if got msg from reciever --> pass the msg as is to sender and close the socket
    //////////////////////////////////////////////////////////////////////////
    bool exit_loop = false;
    int counter = 0;
    while (counter < 3) {
        String string = { "\0", 0 };
        if ((recv_len = recvfrom(receiver_s, str, MAX_CHUNK, 0, (struct sockaddr*)&channel, &slen)) ==
            SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
        }
        counter++;
        string.length = recv_len;
        string.str = str;
        printf("%d\n", recv_len);
        printf("%s\n", string.str);
        String str_in_bits = string_to_binary(string);
        printf("%s\n", str_in_bits.str);
        String final_bits = add_first_bit(str_in_bits, AFTER_HAMMING_SIZE);
        printf("%s\n", final_bits.str);
        String unwrapped = unwrap_hamming(final_bits);
        printf("%s\n", unwrapped.str);
        String bits_to_str = bits_to_string(unwrapped);
        printf("%s\n", bits_to_str.str);
        free(str_in_bits.str);
        free(final_bits.str);
        free(unwrapped.str);
        free(bits_to_str.str);
    }

    char a[] = "good";
    int length = strlen(a);

    if (sendall(receiver_s, a, &length, channel) == EXIT_FAILURE) {
        fprintf(stderr, "Erron in sendall()\n");
    }
    closesocket(receiver_s);
    WSACleanup();

    return EXIT_SUCCESS;
}