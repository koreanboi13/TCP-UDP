#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

struct Message {
    std::string date;
    short AA;
    unsigned int BBB;
    std::string text;
};

std::vector<Message> parse_file(const std::string& filename) {
    std::ifstream file(filename.c_str()); 
    std::string line;
    std::vector<Message> messages;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        Message msg;
        iss >> msg.date >> msg.AA >> msg.BBB;
        std::getline(iss, msg.text);
        if (!msg.text.empty() && msg.text[0] == ' ')
            msg.text.erase(0, 1);
        messages.push_back(msg);
    }

    return messages;
}
void send_message(int sockfd, int index, const Message& msg) {
    unsigned int net_index = htonl(index);
    unsigned int net_date = htonl(std::stoi(msg.date.substr(6, 4)) * 10000 + std::stoi(msg.date.substr(3, 2)) * 100 + std::stoi(msg.date.substr(0, 2)));
    short net_AA = htons(msg.AA);
    unsigned int net_BBB = htonl(msg.BBB);
    unsigned int net_msg_len = htonl(msg.text.length());

    if (send(sockfd, &net_index, sizeof(net_index), 0) == -1) {
        std::cout << "Send failed"<< std::endl;
        return;
    }
    if (send(sockfd, &net_date, sizeof(net_date), 0) == -1) {
        std::cout << "Send failed"<< std::endl;
        return;
    }
    if (send(sockfd, &net_AA, sizeof(net_AA), 0) == -1) {
        std::cout << "Send failed"<< std::endl;
        return;
    }
    if (send(sockfd, &net_BBB, sizeof(net_BBB), 0) == -1) {
        std::cout << "Send failed" << std::endl;
        return;
    }
    if (send(sockfd, &net_msg_len, sizeof(net_msg_len), 0) == -1) {
        std::cout << "Send failed" << std::endl;
        return;
    }
    if (send(sockfd, msg.text.c_str(), msg.text.length(), 0) == -1) {
        std::cout << "Send failed" << std::endl;
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

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);

    for (int attempt = 0; attempt < 10; ++attempt) {
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0) {
            break;
        }
        usleep(100000); // 100ms
        if (attempt == 9) {
            std::cout << "Failed to connect after 10 attempts" << std::endl;
            close(sockfd);
            return 1;
        }
    }

    if (send(sockfd, "put", 3, 0) == -1) {
        std::cout << "Failed to send 'put' command" << std::endl;
        close(sockfd);
        return 1;
    }

    std::vector<Message> messages = parse_file(filename);

    for (int i = 0; i < messages.size(); ++i) {
        send_message(sockfd, i, messages[i]);

        char response[2];
        if (recv(sockfd, response, 2, 0) <= 0) {
            std::cout << "Failed to receive 'ok' from server" << std::endl;
            close(sockfd);
            return 1;
        }
        if (response[0] != 'o' || response[1] != 'k') {
            std::cout << "Unexpected response from server: " << response[0] << response[1] << std::endl;
            close(sockfd);
            return 1;
        }
    }
    close(sockfd);
    return 0;
}
