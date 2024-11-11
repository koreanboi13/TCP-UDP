# TCP/UDP Servers and Clients in C++ (Linux and Windows)

This project demonstrates the implementation of TCP and UDP servers and clients using C++ for both Linux and Windows platforms. The code provides basic examples of how to create both client-server and peer-to-peer communication using TCP and UDP protocols.

## Table of Contents
- [Description](#description)
- [Usage](#features)
- [Compilation](#Compilation)
- [Examples](#examples)

## Description

This project implements both TCP and UDP server and client programs using C++ for two major platforms: Linux and Windows. It aims to showcase the usage of socket programming for creating reliable (TCP) and fast, connectionless (UDP) communication. 

### TCP:
- **Reliable connection**: Ensures data is delivered in order.
- **Flow control and congestion control**: Built-in mechanisms for efficient data transfer.

### UDP:
- **Unreliable connection**: Data is sent without ensuring delivery.
- **Low latency**: Suitable for real-time applications.

## Features
- Example of both TCP and UDP protocols for server-client communication
- Handling of multiple clients
- Simple message exchange between client and server
- Multi-threading support for handling multiple connections (TCP server)

## Compilation

### To compile program on Windows
g++ prog.cpp -o prog -lws2_32

### To compile program on Linux
g++ prog.cpp -o prog

## Examples

### Run TCP client
./tcpclient IP:PORT SAMPLE.TXT

### Run TCP server
./tcpserver PORT

### Run UDP client
./tcpclient IP:PORT SAMPLE.TXT

### Run UDP Server
./udpserver FIRST_PORT - LAST_PORT
