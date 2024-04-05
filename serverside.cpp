#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <filesystem>

///////////////  Computer Networks Project : Hybrid P2P File Sharing and Chatting Application
//////////////   Made by :                   FAIZAN EJAZ (384102)(DE-43,CE-A)
//////////////                               ZAKRIYA MEHMOOD (391449)(DE-43,CE-A)

#define PORT 1234
#define MAX_CLIENTS 1000
#define MAX_BUFFER_SIZE 1024

using namespace std;

struct Client {
    int socket;
    int portnumber;
    std::string ipaddress;
    std::string username;
    std::string sharedDirectory;
    std::vector<std::string> sharedFiles;
};

std::vector<Client> clients;
std::mutex clientsMutex;

void broadcastnewClientEntry(const std::string& targetUsername){
//std::lock_guard<std::mutex> lock(clientsMutex); 
for (const auto& client : clients) {
	    
	    std::cout<<"' "+targetUsername+" ' has entered the our P2P network" <<std::endl; 
            send(client.socket, ("' "+targetUsername+" ' has entered the our P2P network").c_str(), MAX_BUFFER_SIZE, 0);
            std::cout<<" sent broadcast"<<std::endl; //testing code   
    }
}
void shareSharedFilesList(int clientSocket) {
    //std::lock_guard<std::mutex> lock(clientsMutex);
    std::string fileList;
    // Broadcast shared files to requesting client
    for (const auto& client : clients) {   
        for (const auto& file : client.sharedFiles) {
            fileList += client.username + " : " + file + ",";
        }        
    }
    // Send file list to the client
        send(clientSocket, fileList.c_str(), fileList.size(), 0);
        fileList.erase();
        //memset(fileList, '\0', sizeof(fileList));
        //std::cin.ignore();
}

void handleSearchRequest(int clientSocket, const std::string& query) {
    int check=0;
    //char buffer[MAX_BUFFER_SIZE];
    std::vector<std::string> searchResults;
    //std::lock_guard<std::mutex> lock(clientsMutex);
    //recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout<<" \n query (inside function) : "<<query<<std::endl;//testing code
    // Search for files in all clients' shared directories
    for (const auto& client : clients) {
        for (const auto& file : client.sharedFiles) {
            if (file.find(query) != std::string::npos) {
                searchResults.push_back(client.username + ": " + file);
                check=1; //file found
            }
        }
    }

    // Send search results to the client
    std::string resultString;
    if(check==1){ 
        for (const auto& result : searchResults) {
        resultString += result + ",";
        }
        std::cout<<"sent search results "<<std::endl;
        send(clientSocket, resultString.c_str(), resultString.size(), 0);
    }
    else{
        send(clientSocket, " No such file found. ,", MAX_BUFFER_SIZE, 0);
    }
    resultString.erase();
    //memset(resultString, '\0', sizeof(resultString));
}

void sendnamesofallpeers(int clientSocket){
    //std::lock_guard<std::mutex> lock(clientsMutex);
    std::string usernamesList;
    // Broadcast shared files to requesting client
    for (const auto& client : clients) {   
        //for (const auto& file : client.sharedFiles) {
            usernamesList += client.username + ",";
        //}        
    }
    // Send file list to the client
        send(clientSocket, usernamesList.c_str(), usernamesList.size(), 0);
        usernamesList.erase();
        //memset(fileList, '\0', sizeof(fileList));
        //std::cin.ignore();
}
void handleChatOrDownloadRequest(int clientSocket, const std::string& targetUsername) {
   // std::lock_guard<std::mutex> lock(clientsMutex);

    // Search for the target user in the list of clients
    for (const auto& client : clients) {
    //std::cout<<" client username : "<<client.username<<std::endl;
    //std::cout<<" target username : "<<targetUsername<<std::endl;
        if (client.username == targetUsername) {
            //send(clientSocket, "Chat started. Type 'exit' to end.", MAX_BUFFER_SIZE, 0);
		send(clientSocket,client.ipaddress.c_str(),MAX_BUFFER_SIZE,0);//send target ip
		std::string portString=std::to_string(client.portnumber);
		send(clientSocket,portString.c_str(),MAX_BUFFER_SIZE,0);//send target port
                
                
            return;
        }
    }

    // If the target user is not found, send a message indicating that the user is not available
    send(clientSocket, "User not available(ip address not found)", MAX_BUFFER_SIZE, 0);
    send(clientSocket, "User not available(port number not found)", MAX_BUFFER_SIZE, 0);
    
}
void receiveFileNames(int clientSocket, Client& client) {
    size_t numFiles;
    recv(clientSocket, &numFiles, sizeof(numFiles), 0);

    for (size_t i = 0; i < numFiles; ++i) {
        size_t fileNameSize;
        recv(clientSocket, &fileNameSize, sizeof(fileNameSize), 0);

        char fileName[256]; // Assuming a maximum file name size of 255 characters
        recv(clientSocket, fileName, fileNameSize, 0);
        fileName[fileNameSize] = '\0';

        client.sharedFiles.push_back(fileName);
    }
}
void handleClient(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Receive the username from the client
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string username(buffer);
std::cout << username << std::endl;
    //std::lock_guard<std::mutex> lock(clientsMutex);

    // Create a new client and add it to the list
    Client newClient;
    newClient.socket = clientSocket;
    newClient.username = username;

    // Receive name of the shared directory
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string sharedDirectory(buffer);
    newClient.sharedDirectory = sharedDirectory;
std::cout << sharedDirectory << std::endl;
    //memset(buffer, 0, sizeof(buffer));
    
    ////////// receive shared files names from the shared directory
    memset(buffer, 0, sizeof(buffer));
    //Client clientData;
    receiveFileNames(clientSocket, newClient);

    // Display the received file names
    for (const auto& file : newClient.sharedFiles) {
        std::cout << "Received file name: " << file << std::endl;
    }   
    
  // Receive client's public IP for assissting in P2P connections
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::string publicIP(buffer);
    newClient.ipaddress = publicIP;
std::cout << publicIP << std::endl; 
 // Receive client's dedicated port number for assissting in P2P connections
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    int portNUM=std::stoi(buffer);
    newClient.portnumber = portNUM;
std::cout << portNUM << std::endl; 
std::cout << "Sender IP Address: " << publicIP << std::endl;
    std::cout << "Sender Port: " << portNUM << std::endl;
 
    

    // Add the new client to the list
    clients.push_back(newClient);

    // Notify all clients about the new client's entry into the P2P network
    //broadcastnewClientEntry(newClient.username);

    // Implement file search, download, and chat functionalities here
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);

        if (std::string(buffer) == "exit") {
            break;
        }
	ssize_t bytesRead;
        int requestType = atoi(buffer);
        std::cout<<" case : "<<requestType<<std::endl;
        switch (requestType) {
       	    case 1:
                // Share all files list
                //memset(buffer, 0, sizeof(buffer));
                //recv(clientSocket, buffer, sizeof(buffer), 0);
                shareSharedFilesList(clientSocket);
                break;
            case 2:
                // Search for a file
                //memset(buffer, 0, sizeof(buffer));//duplicate to discard unnecessary repetetion of choice num
                //recv(clientSocket, buffer, sizeof(buffer), 0);//duplicate to discard unnecessary repetetion of choice num
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // Handle disconnection or error
                    std::cout << "Client disconnected or error in receiving data.\n";
                    break; // Exit the loop and close the socket
                }
                //recv(clientSocket, buffer, sizeof(buffer), 0);
                std::cout<<"\n query : "<<buffer<<std::endl;//testing code
                handleSearchRequest(clientSocket, buffer);
                break;
            case 3:
                // Download a file
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // Handle disconnection or error
                    std::cout << "Client disconnected or error in receiving data.\n";
                    break; // Exit the loop and close the socket
                }
                handleSearchRequest(clientSocket, buffer);
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // Handle disconnection or error
                    std::cout << "Client disconnected or error in receiving data.\n";
                    break; // Exit the loop and close the socket
                }
                handleChatOrDownloadRequest(clientSocket, buffer);
                //recv(clientSocket, buffer, sizeof(buffer), 0);
                //handleDownloadRequest(clientSocket, buffer);
                break;
            case 4:
                // Chat with a user
                sendnamesofallpeers(clientSocket);
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // Handle disconnection or error
                    std::cout << "Client disconnected or error in receiving data.\n";
                    break; // Exit the loop and close the socket
                }
                //recv(clientSocket, buffer, sizeof(buffer), 0);
                handleChatOrDownloadRequest(clientSocket, buffer);
                break;
            case 0:
            return;
            default:
                std::cout << "Invalid request type." << std::endl;
        }
        
    }

    // Close the client socket when done
    close(clientSocket);
}

int main() {
    int serverSocket, newClientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    std::cout << " Faizan's P2P chating and file sharing Server LIVE. listening for connections on port " << PORT << std::endl;

    while (clients.size() < MAX_CLIENTS) {
        // Accept a new client connection
        newClientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (newClientSocket == -1) {
            perror("Error accepting connection");
            continue;
        }

        // Handle the new client in a separate thread
        std::thread(handleClient, newClientSocket).detach();
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
