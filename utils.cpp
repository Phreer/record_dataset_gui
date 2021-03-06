#include "utils.h"

int sendn(SOCKET sock, void * buffer_ptr, size_t len) {
    size_t n_left = len;
    size_t n_written = 0;
    char *ptr;
    ptr = (char*) buffer_ptr;
    while (n_left > 0) {
        n_written = send(sock, ptr, n_left, 0);
        if ((n_written) == SOCKET_ERROR) {
            printf("sendn: send error. %ld\n", WSAGetLastError());
        }
        n_left -= n_written;
        ptr += n_written;
    }
    return len - n_left;
}
void set_addr_s(struct sockaddr_in *addr, const char *ip, u_short port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr->sin_addr);
}

bool send_command(SOCKET socket, command_t c) {
    strcpy_s(send_buffer, BUFFER_SIZE, COMMAND[c]);
    if (sendn(socket, send_buffer, COMMAND_LEN) != COMMAND_LEN) {
        printf("send_command: send error. %ld\n", WSAGetLastError());
        return false;
    }
    return true;
}

/*
 * transform 64 byte int in network byte order to host byte order
 */
int64_t ntoh64(void *ptr){
    char *cptr = (char*) ptr;
    char tmp_buffer[8];
    for(int i=0; i<8; i++){
        tmp_buffer[7-i] = *(cptr+i);
        printf("%x", *(cptr+i));
    }
    int64_t res = *((int64_t*)tmp_buffer);
    return res;
}
