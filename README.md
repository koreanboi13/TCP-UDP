# TCP/UDP Servers and Clients in C++ (Linux and Windows)

This project demonstrates the implementation of TCP and UDP servers and clients using C++ for both Linux and Windows platforms. The code provides basic examples of how to create both client-server and peer-to-peer communication using TCP and UDP protocols.

## Table of Contents
- [Description](#description)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Examples](#examples)
- [Supported Platforms](#supported-platforms)
- [Contributing](#contributing)
- [License](#license)

## Description

This project implements both TCP and UDP server and client programs using C++ for two major platforms: Linux and Windows. It aims to showcase the usage of socket programming for creating reliable (TCP) and fast, connectionless (UDP) communication. 

### TCP:
- **Reliable connection**: Ensures data is delivered in order.
- **Flow control and congestion control**: Built-in mechanisms for efficient data transfer.

### UDP:
- **Unreliable connection**: Data is sent without ensuring delivery.
- **Low latency**: Suitable for real-time applications.

## Features
- Cross-platform support (Linux and Windows)
- Example of both TCP and UDP protocols for server-client communication
- Handling of multiple clients (for TCP)
- Simple message exchange between client and server
- Multi-threading support for handling multiple connections (TCP server)

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/tcp-udp-servers-clients.git
   cd tcp-udp-servers-clients
