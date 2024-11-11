#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <arpa/inet.h>  
#include <stdlib.h>     
#include <unistd.h>    
#include <ctime>
#include <fstream>
#include <sstream>
#define N 100
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define CLIENT_TIMEOUT 30  

int flag_stop = 0;
struct data_base{
	unsigned int ip;
	unsigned short port;
	int recv_message[20];
	time_t last_activity;
};

int sock_err(const char* function){
	int err;
	err = errno;
	fprintf(stderr, "%s: socket error: %d\n", function, err);
	return -1;
}
int find_client(data_base* clients, int client_count, uint32_t ip, uint16_t port) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].ip == ip && clients[i].port == port) {
            return i;
        }
    }
    return -1;
}
std::string int_to_string(int value) {
    std::stringstream ss;
    ss << value; 
    return ss.str();
}
int set_non_block_mode(int s){
	int fl = fcntl(s, F_GETFL, 0);
	return fcntl(s, F_SETFL, fl | O_NONBLOCK);
}
void remove_inactive_clients(data_base clients[], int& client_count) {
    time_t current_time = time(NULL);
    for (int i = 0; i < client_count;) {
        if (current_time - clients[i].last_activity > CLIENT_TIMEOUT) 
            clients[i] = clients[--client_count];
        else 
        	i++;
    }
}
int handle_message(int client_sockets, struct data_base*clients, int&client_count, sockaddr_in client_addr,std::ofstream& outfile)
{
	char buffer[4096] = { 0 };
	socklen_t  addr_len = sizeof(client_addr);
	int len = 0;
	int rcv = recvfrom(client_sockets,buffer,sizeof(buffer),0,(struct sockaddr*)&client_addr,&addr_len);
	if(rcv <= 0)
		std::cout << "Reciev error" <<std::endl;

	buffer[rcv] = '\0';
    unsigned short client_port = ntohs(client_addr.sin_port);
    unsigned int client_ip_addr = htonl(client_addr.sin_addr.s_addr);
    int client_index = find_client(clients, client_count, client_ip_addr, client_port);
    if (client_index == -1) {
        if (client_count >= MAX_CLIENTS) {
            std::cout << "Max clients reached, cannot add new client." << std::endl;
            return 0;
        }
        client_index = client_count++;
        clients[client_index].ip = client_ip_addr;
        clients[client_index].port = client_port;
 		memset(&clients[client_index].recv_message,0,sizeof(clients[client_index].recv_message));
    }
    clients[client_index].last_activity = time(NULL);
	unsigned int num;
	memcpy(&num, buffer, 4);
	num = htonl(num);
	if(!clients[client_index].recv_message[num]){
		clients[client_index].recv_message[num] = 1;

		unsigned int date_val;
		memcpy(&date_val,buffer+4,4);

		short AA;
		memcpy(&AA,buffer+8,2);

		unsigned int BBB;
		memcpy(&BBB,buffer+10,4);

	    unsigned int len_msg;
	    memcpy(&len_msg,buffer+14,4);

	    len_msg = htonl(len_msg);
	    char msg_buffer[2048];
	    memcpy(&msg_buffer,buffer+18, len_msg);

	    msg_buffer[len_msg] = '\0';
	    
	    date_val = htonl(date_val);
	    AA = htons(AA);
	    BBB = htonl(BBB);
	    std::string date = int_to_string(date_val);
		date = date.substr(6, 2) + "." + date.substr(4, 2) + "." + date.substr(0, 4);

		num = ntohl(num);
	    outfile << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << " ";
	    outfile << date << " " << AA << " " << BBB << " " << std::string(msg_buffer) << std::endl;
	   
	   	
		sendto(client_sockets, (char*)&num, 4, 0, (struct sockaddr*)&client_addr, addr_len);
		if(std::string(msg_buffer)== "stop"){
	   		flag_stop = 1;
	   		return 1;
	   	}
	}
	else
		return 0;
}
int main(int argc, char**argv){
	if(argc != 3){
		std::cout << "Usage: " << argv[0] << " <start_port> <end_port>" << std::endl;
        return 1;
	}
	unsigned int start_port = atoi(argv[1]), end_port = atoi(argv[2]);
	unsigned int port_count = end_port - start_port + 1;
	int client_sockets[N];
	struct pollfd pfd[N];
	struct sockaddr_in addr[N];
	struct data_base clients[N];
	int client_count = 0;
	for (int port = start_port; port <= end_port; ++port) {
        client_sockets[port - start_port] = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_sockets[port - start_port] < 0) {
            std::cout << "Error creating socket on port " << port << std::endl;
            return 1;
        }

        set_non_block_mode(client_sockets[port - start_port]);

    	memset(&addr[port-start_port],0,sizeof(addr[port-start_port]));
        addr[port-start_port].sin_family = AF_INET;
        addr[port-start_port].sin_addr.s_addr = htonl(INADDR_ANY);
        addr[port-start_port].sin_port = htons(port);

        if (bind(client_sockets[port - start_port], (struct sockaddr*)&addr[port-start_port], sizeof(addr[port-start_port])) < 0) {
            std::cout << "Error binding to port " << port << std::endl;
            close(client_sockets[port - start_port]);
            return 1;
        }

        std::cout << "Listening on port " << port << std::endl;

        pfd[port - start_port].fd = client_sockets[port - start_port];
        pfd[port - start_port].events = POLLIN; 
    }
    std::ofstream outfile("msg.txt", std::ios::app);
    while(1)
    {
    	remove_inactive_clients(clients,client_count);
    	int ev_cnt = poll(pfd, sizeof(pfd)/sizeof(pfd[0]),1000);
    	if(ev_cnt>0){
    		for(int i=0;i < port_count;i++){
    			if(pfd[i].revents & POLLIN){
    				
    				if(handle_message(client_sockets[i],clients,client_count,addr[i],outfile))
    					break;
    			}
    		}
    		if(flag_stop){
    			for(int i = 0;i<port_count;i++)
    				close(client_sockets[i]);
    			std::cout << "'Stop' message recieved. Terminating...." <<std::endl;
    			outfile.close();
    			exit(0);
    		}

    	}
    }
}