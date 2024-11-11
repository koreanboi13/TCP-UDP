#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#define N 50
struct Message {
    unsigned int index;
    std::string date;
    short AA;
    unsigned int BBB;
    std::string text;
};
int len_sockets = 0;
int client_sockets[N];
std::vector<Message> messages;

int is_valid_date(const std::string& date) {
    if (date.length() != 10 || date[2] != '.' || date[5] != '.') {
        return 0;
    }

    int day = std::stoi(date.substr(0, 2));
    int month = std::stoi(date.substr(3, 2));
    int year = std::stoi(date.substr(6, 4));

    if (day < 1 || day > 31 || month < 1 || month > 12 || year < 1900 || year > 2100)
        return 0;

    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (day > (leap ? 29 : 28))
            return 0;
    } 
    else if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30)
            return 0;
    }
    return 1;
}
bool is_valid_AA(int AA) {
    return (AA >= -32768 && AA <= 32767);
}
bool is_valid_BBB(unsigned int BBB) {
    return (BBB <= 4294967295);
}
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
        int fourth_space = line.find(' ', third_space+1);

        msg.date = line.substr(first_space + 1, second_space - first_space - 1); 
        msg.AA = std::stoi(line.substr(second_space + 1, third_space - second_space - 1)); 
        msg.BBB = std::stoul(line.substr(third_space + 1, fourth_space - third_space - 1)); 
        msg.text = line.substr(fourth_space + 1); 
        msg.index = messages.size();  
        messages.push_back(msg);
    }

    return messages;
}
void handle_client(SOCKET client_socket, sockaddr_in client_addr) {
    char buffer[8192];
    int bytes_received;

    while (true) {
        unsigned int  net_index, net_date, net_BBB, net_msg_len;
        short  net_AA;

        bytes_received = recv(client_socket, (char*)&net_index, sizeof(net_index), 0);
        if (bytes_received <= 0) break;

        
        bytes_received = recv(client_socket, (char*)&net_date, sizeof(net_date), 0);
        if (bytes_received <= 0) break;

        
        bytes_received = recv(client_socket, (char*)&net_AA, sizeof(net_AA), 0);
        if (bytes_received <= 0) break;

        
        bytes_received = recv(client_socket, (char*)&net_BBB, sizeof(net_BBB), 0);
        if (bytes_received <= 0) break;

        
        bytes_received = recv(client_socket, (char*)&net_msg_len, sizeof(net_msg_len), 0);
        if (bytes_received <= 0) break;

        unsigned int  index = ntohl(net_index);
        unsigned int  date_val = ntohl(net_date);
        short  AA = ntohs(net_AA);
        unsigned int BBB = ntohl(net_BBB);
        unsigned int  msg_len = ntohl(net_msg_len);

        bytes_received = recv(client_socket, buffer, msg_len, 0);
        if (bytes_received <= 0) break;

        buffer[bytes_received] = '\0';  

        std::string date = std::to_string((long long)date_val);
        date = date.substr(6, 2) + "." + date.substr(4, 2) + "." + date.substr(0, 4);

        std::ofstream outfile("msg.txt", std::ios::app);
        outfile << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << " ";
        outfile << date << " " << AA << " " << BBB << " " << std::string(buffer) << std::endl;
        outfile.close();

        
        send(client_socket, "ok", 2, 0);

        
        if (std::string(buffer) == "stop") {
            for (int i = 0; i < len_sockets; i++) {
                SOCKET s = client_sockets[i];
                closesocket(s);  
            }
            WSACleanup();
            exit(0);  
        }
    }
    closesocket(client_socket);
}
void handle_get_request(SOCKET sockfd) {
    messages = parse_file("msg.txt");
    if(messages.size() == 0)
        closesocket(sockfd);
    for(int i = 0; i < messages.size();i++)
    {
        if(is_valid_date(messages[i].date) && is_valid_AA(messages[i].AA) && is_valid_BBB(messages[i].BBB)){
            unsigned int net_index = htonl(i);
            unsigned int net_date = htonl(std::stoi(messages[i].date.substr(6, 4)) * 10000 + std::stoi(messages[i].date.substr(3, 2)) * 100 + std::stoi(messages[i].date.substr(0, 2)));
            short net_AA = htons(messages[i].AA);
            unsigned int net_BBB = htonl(messages[i].BBB);
            unsigned int net_msg_len = htonl(messages[i].text.length());
            if (send(sockfd, (char*)&net_index, sizeof(net_index), 0) == -1) {
                std::cout << "Send failed"<< std::endl;
                return;
            }
            if (send(sockfd, (char*)&net_date, sizeof(net_date), 0) == -1) {
                std::cout << "Send failed"<< std::endl;
                return;
            }
            if (send(sockfd, (char*)&net_AA, sizeof(net_AA), 0) == -1) {
                std::cout << "Send failed"<< std::endl;
                return;
            }
            if (send(sockfd, (char*)&net_BBB, sizeof(net_BBB), 0) == -1) {
                std::cout << "Send failed" << std::endl;
                return;
            }
            if (send(sockfd, (char*)&net_msg_len, sizeof(net_msg_len), 0) == -1) {
                std::cout << "Send failed" << std::endl;
                return;
            }
            if (send(sockfd, messages[i].text.c_str(), messages[i].text.length(), 0) == -1) {
                std::cout << "Send failed" << std::endl;
                return;
            }
        }
    }
    closesocket(sockfd);
}
int set_non_block_mode(int s)
{
    unsigned long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    int server_port = atoi(argv[1]);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(client_sockets,-1,sizeof(client_sockets));
    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(server_port);

    if (bind(server_socket, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, N) < 0) {
        std::cout << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << server_port << std::endl;

    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        int max_sd = server_socket;

        for (int i = 0; i < N; i++) {
            if (client_sockets[i] > 0)
                FD_SET(client_sockets[i], &readfds);

            if (client_sockets[i] > max_sd)
                max_sd = client_sockets[i];
        }
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity == SOCKET_ERROR) {
            std::cout << "Select error: " << WSAGetLastError() << std::endl;
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

        if (FD_ISSET(server_socket, &readfds)) {
            int client_len = sizeof(serv_addr);
            client_sockets[len_sockets] = accept(server_socket, (sockaddr*)&serv_addr, &client_len);
            set_non_block_mode(server_socket);
            if (client_sockets[len_sockets] < 0) {
                std::cout << "Accept error: " << WSAGetLastError() << std::endl;
                closesocket(server_socket);
                WSACleanup();
                return 1;
            }
            len_sockets++;   
            
        }
        for (int i = 0; i < N; i++) {
            if (client_sockets[i] > 0 && FD_ISSET(client_sockets[i], &readfds)) {
                char client_type[3];
                recv(client_sockets[i], client_type, 3, 0);
                            
                if (strncmp(client_type, "put", 3) == 0)
                    handle_client(client_sockets[i], serv_addr);
                else if(strncmp(client_type,"get",3) == 0)
                    handle_get_request(client_sockets[i]);
                else {
                    std::cout << "Unknown client request type: " << client_type << std::endl;
                    closesocket(client_sockets[i]);
                }
                client_sockets[i] = -1;
                len_sockets--;
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
