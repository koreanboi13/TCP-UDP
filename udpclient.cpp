#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

struct Message {
    unsigned int index;
    std::string date;
    short AA;
    unsigned int BBB;
    std::string text;
};
struct Messages_status{
    int messages_status[20];
    int messages_count;
    int messages_sent;
}; 
std::vector<Message> messages;
std::vector<Message> parse_file(const std::string& filename) {
    std::ifstream file(filename.c_str());
    std::string line;
    std::vector<Message> messages;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Message msg;
        int first_space = line.find(' '); 
        int second_space = line.find(' ', first_space + 1); 
        int third_space = line.find(' ', second_space + 1); 

        msg.date = line.substr(0, first_space); 
        msg.AA = std::stoi(line.substr(first_space + 1, second_space - first_space - 1)); 
        msg.BBB = std::stoul(line.substr(second_space + 1, third_space - second_space - 1)); 
        msg.text = line.substr(third_space + 1); 
        msg.index = messages.size();
        messages.push_back(msg);
    }

    return messages;
}

int send_message(int s, struct Message* msg, struct sockaddr_in addr) {
    char buffer[1024] = { 0 };

    unsigned int net_message_num = htonl(msg->index);
    unsigned int net_date = htonl(std::stoi(msg->date.substr(6, 4)) * 10000 + std::stoi(msg->date.substr(3, 2)) * 100 + std::stoi(msg->date.substr(0, 2)));
    short net_AA = htons(msg->AA);
    unsigned int net_BBB = htonl(msg->BBB);
    unsigned int msg_len = htonl(msg->text.length());

    
    memcpy(buffer, &net_message_num, sizeof(net_message_num));
    memcpy(buffer + sizeof(net_message_num), &net_date, sizeof(net_date));
    memcpy(buffer + sizeof(net_message_num) + sizeof(net_date), &net_AA, sizeof(net_AA));
    memcpy(buffer + sizeof(net_message_num) + sizeof(net_date) + sizeof(net_AA), &net_BBB, sizeof(net_BBB));
    memcpy(buffer + sizeof(net_message_num) + sizeof(net_date) + sizeof(net_AA) + sizeof(net_BBB), &msg_len, sizeof(msg_len));
    memcpy(buffer + sizeof(net_message_num) + sizeof(net_date) + sizeof(net_AA) + sizeof(net_BBB) + sizeof(msg_len), (msg->text.c_str()), msg->text.length()+1);

    int r = sendto(s, buffer, sizeof(net_message_num) + sizeof(net_date) + sizeof(net_AA) + sizeof(net_BBB) + sizeof(msg_len) + msg->text.length(), 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (r < 0)
        std::cout<<"Sendto error: " << WSAGetLastError << std::endl;

    return 0;
}

void recv_response(int socket, Messages_status* msg){
    struct timeval tv = {0, 100 * 1000};
    int res;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    res = select(socket+1, &fds, NULL,NULL,&tv);
    if(res>0)
    {
        struct sockaddr_in addr;
        char datagram[20];
        int len = sizeof(addr);
        int rec =recvfrom(socket,datagram,sizeof(datagram),0,(struct sockaddr*)&addr, &len);
        if(rec > 0){
            for (int j = 0; j < rec / 4 && j < 20; j++) {
                unsigned int num;
                memcpy(&num, datagram + j * 4, 4);
                num = ntohl(num);

                if (!msg->messages_status[num])
                    msg->messages_sent++;
                msg->messages_status[num] = 1;
            }
        }   
        else{
            std::cout<< "Recvfrom error: "<<WSAGetLastError() << std::endl;
            return;
        }
    }
    else if(res == 0)
        return;
    else{
        std::cout<< "Select error: " << WSAGetLastError() << std::endl;
        return;
    }

}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <server_ip:port> <filename>" << std::endl;
        return 1;
    }

    std::string server_ip = strtok(argv[1], ":");
    int server_port = atoi(strtok(NULL, ":"));
    std::string filename = argv[2];

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    struct sockaddr_in addr;
    memset(&addr, 0 , sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    addr.sin_port = htons(server_port);
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    messages = parse_file(filename);
    std::cout<<messages[0].text<<std::endl;
    Messages_status status;
    status.messages_count = messages.size();
    status.messages_sent = 0;
    memset(&status.messages_status, 0, sizeof(status.messages_status));
    
    std::cout<< "Message count: " << status.messages_count << std::endl;
    while(status.messages_sent < 20 && status.messages_sent != status.messages_count){
        for(int i = 0; i<status.messages_count;i++){
            if(!status.messages_status[i])
                send_message(sockfd,&messages[i], addr);
        }
        recv_response(sockfd,&status);
    }
    closesocket(sockfd); 
    WSACleanup();
    return 0;
}