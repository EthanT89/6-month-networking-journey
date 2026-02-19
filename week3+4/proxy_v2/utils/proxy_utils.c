#include "./proxy_utils.h"

int prepend_address(struct sockaddr *addr, unsigned char packet[MAXBUFSIZE], unsigned char buf[MAXBUFSIZE], size_t buflen){
    int offset = 0;
    
    memcpy(packet+offset, addr->sa_data, sizeof (addr->sa_data)); offset += sizeof(addr->sa_data);
    memcpy(packet+offset, &addr->sa_family, sizeof (addr->sa_family)); offset += sizeof(addr->sa_family);
    memcpy(packet+offset, buf, buflen); offset += buflen;

    return offset;
}

int send_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, int flags, struct sockaddr *addr, socklen_t addr_len){
    
    unsigned char packet[MAXBUFSIZE];
    int packetlen = buflen;
    int bytes_sent;
    int offset = 0;

    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT_N); // Proxy port
    inet_pton(AF_INET, "127.0.0.1", &proxy_addr.sin_addr); // Proxy IP 127.0.0.1

    offset = prepend_address(addr, packet, buf, buflen);

    // printf("packed %d bytes for proxy\n", offset);

    if ((bytes_sent = sendto(sockfd, packet, offset, 0, (struct sockaddr*)&proxy_addr, sizeof(proxy_addr))) == -1){
        return -1;
    }

    return bytes_sent;
}



int rec_proxy(int sockfd, unsigned char buf[MAXBUFSIZE], size_t buflen, int flags, struct sockaddr *addr, socklen_t *addr_len){

    unsigned char packet[MAXBUFSIZE];
    int bytes_received;

    struct sockaddr_in proxy_addr;
    socklen_t proxy_addr_len = sizeof(proxy_addr);
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT_N); // Proxy port
    inet_pton(AF_INET, "127.0.0.1", &proxy_addr.sin_addr); // Proxy IP 127.0.0.1

    if ((bytes_received = recvfrom(sockfd, packet, MAXBUFSIZE, flags, (struct sockaddr*)&proxy_addr, &proxy_addr_len)) == -1){
        return -1;
    }

    // Extract sender address
    memcpy(addr->sa_data, packet, sizeof(addr->sa_data));
    memcpy(&addr->sa_family, packet+sizeof(addr->sa_data), sizeof(addr->sa_family));
    *addr_len = sizeof(*addr);

    memcpy(buf, packet+(*addr_len), bytes_received-(*addr_len));
    return bytes_received - *addr_len;
}
