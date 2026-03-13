/*
 * proxy_config.h -- configuration for the UDP Proxy server
 */

#ifndef PROXY_CONFIG_H
#define PROXY_CONFIG_H

// Standard Global Configuration
#define PROXY_PORT_S "5050" // String value for proxy port
#define PROXY_PORT_N 5050 // Numerical value for the proxy port (saves us converting to string or vice versa "oh no!")

#define DELAY_MS 50 // latency delay in milliseconds
#define DROP_RATE 10 // drop rate percent (eg 5 = 5% of packets are dropped)

#endif