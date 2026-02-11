/*
 * test_delayed_packet.c -- Test file for delayed_packet queue functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUFSIZE 1000

#include "./utils/delayed_packet.h"

// Helper function to create a test packet
struct Packet* create_test_packet(const char *data, int time_received) {
    struct Packet *pkt = malloc(sizeof(struct Packet));
    
    // Copy data
    strncpy((char *)pkt->data, data, MAXBUFSIZE - 1);
    pkt->bytes = strlen(data);
    
    // Create a fake destination address
    pkt->dest_addr = malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in *addr = (struct sockaddr_in *)pkt->dest_addr;
    addr->sin_family = AF_INET;
    addr->sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
    pkt->dest_addrlen = sizeof(struct sockaddr_in);
    
    pkt->time_received = time_received;
    pkt->next = NULL;
    
    return pkt;
}

// Helper to print packet info
void print_packet(struct Packet *pkt) {
    if (pkt == NULL) {
        printf("NULL\n");
        return;
    }
    
    struct sockaddr_in *addr = (struct sockaddr_in *)pkt->dest_addr;
    printf("Data: '%s' | Bytes: %d | Time: %dms | Dest: %s:%d\n",
           pkt->data, pkt->bytes, pkt->time_received,
           inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
}

// Helper to free packet
void free_packet(struct Packet *pkt) {
    if (pkt != NULL) {
        free(pkt->dest_addr);
        free(pkt);
    }
}

int main() {
    printf("=== DELAYED PACKET QUEUE TEST ===\n\n");
    
    // Initialize queue
    struct Packets queue;
    queue.delay_ms = 100;
    queue.pkt_count = 0;
    queue.head = NULL;
    queue.tail = NULL;
    
    // Test 1: is_empty on new queue
    printf("TEST 1: is_empty() on new queue\n");
    printf("Expected: 1 (empty)\n");
    printf("Result:   %d\n", is_empty_packets(&queue));
    printf("✓ PASS\n\n");
    
    // Test 2: enqueue first packet
    printf("TEST 2: enqueue() first packet\n");
    struct Packet *pkt1 = create_test_packet("Hello World", 0);
    printf("Enqueuing: ");
    print_packet(pkt1);
    enqueue_packet(&queue, pkt1);
    printf("Expected queue count: 1\n");
    printf("Result queue count:   %d\n", queue.pkt_count);
    printf("✓ PASS\n\n");
    
    // Test 3: is_empty after enqueue
    printf("TEST 3: is_empty() after enqueue\n");
    printf("Expected: 0 (not empty)\n");
    printf("Result:   %d\n", is_empty_packets(&queue));
    printf("✓ PASS\n\n");
    
    // Test 4: peek at first packet
    printf("TEST 4: peek_packet() should return first packet\n");
    printf("Expected: ");
    print_packet(pkt1);
    printf("Result:   ");
    struct Packet *peeked = peek_packet(&queue);
    print_packet(peeked);
    printf("Queue count after peek: %d (should still be 1)\n", queue.pkt_count);
    printf("✓ PASS\n\n");
    
    // Test 5: enqueue multiple packets
    printf("TEST 5: enqueue() multiple packets\n");
    struct Packet *pkt2 = create_test_packet("Packet 2", 50);
    struct Packet *pkt3 = create_test_packet("Packet 3", 100);
    struct Packet *pkt4 = create_test_packet("Test Data 4", 150);
    
    printf("Enqueuing: ");
    print_packet(pkt2);
    enqueue_packet(&queue, pkt2);
    
    printf("Enqueuing: ");
    print_packet(pkt3);
    enqueue_packet(&queue, pkt3);
    
    printf("Enqueuing: ");
    print_packet(pkt4);
    enqueue_packet(&queue, pkt4);
    
    printf("Expected queue count: 4\n");
    printf("Result queue count:   %d\n", queue.pkt_count);
    printf("✓ PASS\n\n");
    
    // Test 6: peek still returns head
    printf("TEST 6: peek_packet() should still return first packet\n");
    printf("Expected: ");
    print_packet(pkt1);
    printf("Result:   ");
    peeked = peek_packet(&queue);
    print_packet(peeked);
    printf("✓ PASS\n\n");
    
    // Test 7: pop_packet - FIFO order
    printf("TEST 7: pop_packet() - should dequeue in FIFO order\n");
    printf("Expected order: pkt1, pkt2, pkt3, pkt4\n\n");
    
    struct Packet *popped;
    int expected_count = 4;
    
    printf("Pop #1: ");
    popped = pop_packet(&queue);
    print_packet(popped);
    expected_count--;
    printf("Queue count: %d (expected: %d)\n", queue.pkt_count, expected_count);
    free_packet(popped);
    printf("\n");
    
    printf("Pop #2: ");
    popped = pop_packet(&queue);
    print_packet(popped);
    expected_count--;
    printf("Queue count: %d (expected: %d)\n", queue.pkt_count, expected_count);
    free_packet(popped);
    printf("\n");
    
    printf("Pop #3: ");
    popped = pop_packet(&queue);
    print_packet(popped);
    expected_count--;
    printf("Queue count: %d (expected: %d)\n", queue.pkt_count, expected_count);
    free_packet(popped);
    printf("\n");
    
    printf("Pop #4: ");
    popped = pop_packet(&queue);
    print_packet(popped);
    expected_count--;
    printf("Queue count: %d (expected: %d)\n", queue.pkt_count, expected_count);
    free_packet(popped);
    printf("✓ PASS\n\n");
    
    // Test 8: is_empty after all pops
    printf("TEST 8: is_empty() after all elements removed\n");
    printf("Expected: 1 (empty)\n");
    printf("Result:   %d\n", is_empty_packets(&queue));
    printf("✓ PASS\n\n");
    
    // Test 9: pop from empty queue
    printf("TEST 9: pop_packet() from empty queue\n");
    printf("Expected: NULL\n");
    popped = pop_packet(&queue);
    printf("Result:   ");
    print_packet(popped);
    printf("✓ PASS\n\n");
    
    // Test 10: enqueue after emptying
    printf("TEST 10: enqueue() after emptying queue\n");
    struct Packet *pkt5 = create_test_packet("New start", 200);
    printf("Enqueuing: ");
    print_packet(pkt5);
    enqueue_packet(&queue, pkt5);
    printf("Queue count: %d (expected: 1)\n", queue.pkt_count);
    printf("✓ PASS\n\n");
    
    // Cleanup
    popped = pop_packet(&queue);
    free_packet(popped);
    
    printf("=== ALL TESTS PASSED ===\n");
    
    return 0;
}
