#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <atomic>
#include <sys/types.h>
#include <sys/wait.h>
#include <filesystem>

#define PORT 4242 

void single_client_instance() 
{
    
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return; 
    } 

    int good_responses = 0;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(true)
    {
        if(std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) 
            break;
        
        send(sock , hello , strlen(hello) , 0 ); 
        valread = read( sock , buffer, 1024);
        ++good_responses;
    }

    std::ofstream o;
    std::string f_name("child_results/");
    o.open(f_name + std::to_string((int)getpid()));
    o << good_responses;
    o.close();
}
   
int main(int argc, char const *argv[]) 
{ 
    size_t connection_amount = atoi(argv[1]);
    pid_t* pids = new pid_t[connection_amount];
    std::string out_dir = "./child_results";
    
    // Prepare output directory
    std::filesystem::remove_all(out_dir.c_str());
    std::filesystem::create_directory(out_dir.c_str());

    printf("Forking %u processes... Father is %u\n", connection_amount, getpid());
    for (size_t i = 0; i < connection_amount; ++i) {
        pids[i]= fork();
        if (pids[i] == 0) {
            printf("New process! PID = %u\n", getpid());
            single_client_instance();
            exit(0);
        } else {
            printf("Parent! = %u\n", getpid());
        }
    }

    // Wait for all processes...
    for (int i = 0; i < connection_amount; ++i) {
        int status;
        while (-1 == waitpid(pids[i], &status, 0));
    }

    // Calculate test avarage...
    namespace fs = std::filesystem;
    const fs::path pathToShow{ argc >= 2 ? argv[1] : fs::current_path() };
    
    size_t sum = 0;
    for (const auto& entry : fs::directory_iterator(out_dir.c_str())) {
        const auto filenameStr = entry.path().filename().string();
        if (entry.is_regular_file()) {
            std::fstream myfile(entry.path().string(), std::ios_base::in);
            size_t res = 0;
            myfile >> res;
            sum += res;
        }
    }

    printf("Avg %d\n", sum / connection_amount);
    return 0; 
} 