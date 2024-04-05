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
#include <condition_variable>
#include <chrono>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <filesystem>

///////////////  Computer Networks Project : Hybrid P2P File Sharing and Chatting Application
//////////////   Made by :                   FAIZAN EJAZ (384102)(DE-43,CE-A)
//////////////                               ZAKRIYA MEHMOOD (391449)(DE-43,CE-A)

#define PORT 1234
//#define myPort 1050
#define MAX_BUFFER_SIZE 1024
 bool exitRequested=false;
//using namespace std;

struct Client {
    int socket;
    std::string username;
    std::string sharedDirectory;
    std::vector<std::string> sharedFiles;
};

std::mutex mutex;

void receiveFilesNames(int clientSocket) {
    std::vector<std::string> sharedFiles;
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Receive shared files from the server
    recv(clientSocket, buffer, sizeof(buffer), 0);

    std::string fileString(buffer);
    size_t pos = 0;
    while ((pos = fileString.find(",")) != std::string::npos) {
        //std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Shared File: " << fileString.substr(0, pos) << std::endl;
        sharedFiles.push_back(fileString.substr(0, pos));
        fileString.erase(0, pos + 1);
    }

    //return sharedFiles;
}

void sendFileNames(int clientSocket, const std::vector<std::string>& files) {
    size_t numFiles = files.size();
    send(clientSocket, &numFiles, sizeof(numFiles), 0);

    for (const auto& file : files) {
        size_t fileNameSize = file.size();
        send(clientSocket, &fileNameSize, sizeof(fileNameSize), 0);
        send(clientSocket, file.c_str(), fileNameSize, 0);
    }
}
void receiveUsernames(int clientSocket){
    std::vector<std::string> sharedFiles;
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Receive usernames from the server
    recv(clientSocket, buffer, sizeof(buffer), 0);

    std::string fileString(buffer);
    size_t pos = 0;
    while ((pos = fileString.find(",")) != std::string::npos) {
        //std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "User name : " << fileString.substr(0, pos) << std::endl;
        sharedFiles.push_back(fileString.substr(0, pos));
        fileString.erase(0, pos + 1);
    }
}
void searchFile(int clientSocket) {
    // Send search request to the server
    //send(clientSocket, "2", 1, 0);  // "2" is the request type for file search
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    std::cout << "Enter the file you want to search for: ";
    std::cin.ignore();
    //std::cin.getline(buffer, MAX_BUFFER_SIZE);   
    std::cin>>buffer;  
    std::cout<<"\n u want to search : "<<buffer<<std::endl;
    send(clientSocket, buffer, strlen(buffer), 0);

    // Receive search results from the server
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0); //receiveFiles(clientSocket);
    std::cout<<"searched : "<<buffer<<std::endl;//testing code
    std::vector<std::string> searchResults;
    std::string fileString(buffer);
    size_t pos = 0;
    while ((pos = fileString.find(",")) != std::string::npos) {
        //std::lock_guard<std::mutex> lock(consoleMutex);
        //std::cout << "Shared File: " << fileString.substr(0, pos) << std::endl;
        searchResults.push_back(fileString.substr(0, pos));
        fileString.erase(0, pos + 1);
    }
    if (searchResults.empty()) {
        std::cout << "No matching files found." << std::endl;
    } else {
        std::cout << "Search results:" << std::endl;
        for (const auto& result : searchResults) {
            std::cout << result << std::endl;
        }
    }
    memset(buffer, '\0', sizeof(buffer));
}

void sendFile(const std::string& fileName, int otherclientSocket) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Send file size
    send(otherclientSocket, &fileSize, sizeof(fileSize), 0);

    // Send file content
    char buffer[1024];
    while (!file.eof() && fileSize>0) {
        std::streamsize bytesRead = file.read(buffer, sizeof(buffer)).gcount();
        send(otherclientSocket, buffer, file.gcount(), 0);
        fileSize -= bytesRead;
    }

    file.close();
}
void receiveFile(const std::string& fileName, int peerSocket) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file) {
        std::cerr << "Error creating file: " << fileName << std::endl;
        return;
    }

    // Receive file size
    std::streamsize fileSize;
    recv(peerSocket, &fileSize, sizeof(fileSize), 0);

    // Receive and write file content
    char buffer[1024];
    while (fileSize > 0) {
        std::streamsize bytesRead = recv(peerSocket, buffer, sizeof(buffer), 0);
        file.write(buffer, bytesRead);
        fileSize -= bytesRead;
    }

    file.close();
}

void downloadFile(int clientSocket, int peerSocket, std::string mydirectory) {
    char buffer[MAX_BUFFER_SIZE];
    char filenamebuffer[MAX_BUFFER_SIZE];
    std::cout << "Enter the file you want to download: ";
    memset(buffer, 0, sizeof(buffer));
    std::cin.ignore();
    //std::cin.getline(buffer, MAX_BUFFER_SIZE);   
    std::cin>>buffer;  
    std::cout<<"\n u want to download : "<<buffer<<std::endl;
    strcpy(filenamebuffer, buffer);

    send(clientSocket, buffer, strlen(buffer), 0);

    // Receive search results from the server
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0); //receiveFiles(clientSocket);
    std::cout<<"searched : "<<buffer<<std::endl;//testing code
    std::vector<std::string> searchResults;
    std::string fileString(buffer);
    size_t pos = 0;
    while ((pos = fileString.find(",")) != std::string::npos) {
        //std::lock_guard<std::mutex> lock(consoleMutex);
        //std::cout << "Shared File: " << fileString.substr(0, pos) << std::endl;
        searchResults.push_back(fileString.substr(0, pos));
        fileString.erase(0, pos + 1);
    }
    if (searchResults.empty()) {
        std::cout << "No matching files found." << std::endl;
    } else {
        std::cout << "Search results:" << std::endl;
        for (const auto& result : searchResults) {
            std::cout << result << std::endl;
        }
        //char buffer[MAX_BUFFER_SIZE];
        char usernamebuffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        memset(usernamebuffer, 0, sizeof(usernamebuffer));
        
        //std::string username;
        std::cout << "Enter the username from who you want to download the required file : ";
        std::cin.ignore();
        std::cin>>usernamebuffer;
        //std::getline(std::cin, username);
        std::cout<<"\n u entered username : "<<usernamebuffer<<std::endl;//testing code
        // Send the target username to the server
        send(clientSocket, usernamebuffer, sizeof(usernamebuffer), 0);
        memset(buffer, 0, sizeof(buffer));
    
        //std::cout<<"\n "<<buffer<<std::endl;
        ////////////////////////////////////get target ip and port from the server
        recv(clientSocket,buffer,sizeof(buffer),0);
        sockaddr_in peerAddress;
        // Set the address family and port
        peerAddress.sin_family = AF_INET;
        //peerAddress.sin_port = htons(81);
        std::cout<<" Received otherperson's IP : "<<buffer<<std::endl;//testing code. display ip of otherperson
        int check=0;
        // Convert the IP address from string to binary form
        if (inet_pton(AF_INET, buffer, &(peerAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid IP address\n";
        check=1;
        // Handle error
        }
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket,buffer,sizeof(buffer),0);
        std::cout<<" Received otherperson's Port number : "<<buffer<<std::endl;//testing code. display portnum of otherperson
        //std::cout<<"\n "<<buffer<<std::endl;
        if(check==0){
        int portnum = std::stoi(buffer);
    
        peerAddress.sin_port = htons(portnum);
        //peerAddress.sin_addr.s_addr = INADDR_ANY;///////testing
        // Connect to the peer directly now
        //int check1=3;
        if (connect(peerSocket, (struct sockaddr*)&peerAddress, sizeof(peerAddress)) == -1) {
        	perror("Error connecting to peer");
        	//exit(EXIT_FAILURE);
            return;
    		}  
        send(peerSocket, "sendmefile", MAX_BUFFER_SIZE, 0);//send my target peer, a signal to send me a file which im gonna request
        send(peerSocket, filenamebuffer, sizeof(filenamebuffer)-1,0); //send file name to the peer so that they can send me that file
        // Example: File path to save
        std::string filePath = mydirectory+std::string(filenamebuffer);
        std::cout<<filePath<<std::endl;
        // Receive the contents of the file from the client
        receiveFile(filePath, peerSocket);
        std::cout<<" File ' "<<filenamebuffer<<" ' received successfully !"<<std::endl;
        close(peerSocket);
        return;
        }
    }
    memset(buffer, '\0', sizeof(buffer));
}
void sendThreadInitiator(int peerSocket, std::string username){		 
	while(true){   
        if (exitRequested) {
                std::cout << "\nconnection was closed !" << std::endl;
                if(peerSocket!=-1){
                    if (close(peerSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    peerSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                break;
                return;
        }else{
                std::cout << "Enter Message for '" << username << "': " << std::endl;
                std::string message;
                std::getline(std::cin, message);            
                send(peerSocket, message.c_str(), message.size(), 0);
                if (message == "exit") {
                    std::cout << "\nYou closed the connection!" << std::endl;
                    exitRequested = true;
                    if(peerSocket!=-1){
                        if (close(peerSocket) == 0) {
                        // Mark the descriptor as closed by setting it to -1
                        peerSocket = -1;
                        std::cout<<" Socket closed "<<std::endl;
                        } else {
                        std::cerr << "Error closing socket. Socket already closed." << std::endl;
                        }
                    }
                    break;
                    return;
                }  
        }
    }
}
void receiveThreadInitiator(int peerSocket, std::string username){
        char buffer[MAX_BUFFER_SIZE];
	while(true){ 
        if (exitRequested) {
                //std::cout << "\nConnection was closed by yourself '" << username << "'!" << std::endl;
                if(peerSocket!=-1){
                    if (close(peerSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    peerSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                break;
                return;
        }
        else{
            memset(buffer, 0, sizeof(buffer));
            int recbytes = recv(peerSocket, buffer, sizeof(buffer), 0);
            std::string str(buffer);

            if (str == "exit") {
                std::cout << "\nConnection was closed by '" << username << "'!" << std::endl;
                std::cout << "Enter 'exit' to close your sending thread too !" << std::endl;
                exitRequested = true;
                send(peerSocket, "exitReciprocated", MAX_BUFFER_SIZE, 0);
                //close(otherclientSocket);
                //break;
                //return;
            } 
            else if(str == "exitReciprocated"){
                exitRequested=true;
                break;
                return;
            }
            else if (recbytes <= 0 ) {
                exitRequested = true;
                if(peerSocket!=-1){
                    if (close(peerSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    peerSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                break;
                return;
            }else{
                std::cout << "Received Message from '" << username << "': " << buffer << std::endl;
                std::cout << "Enter Message for '" << username << "': " << std::endl;
            }
        }
    }
            
}
void chatWithUser(int clientSocket, int peerSocket, std::string myname) {
    char buffer[MAX_BUFFER_SIZE];
    char usernamebuffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    memset(usernamebuffer, 0, sizeof(usernamebuffer));
    //std::string username;
    std::cout << "Enter the username you want to chat with: ";
    std::cin.ignore();
    std::cin>>usernamebuffer;
    //std::getline(std::cin, username);
    std::cout<<"\n u entered username : "<<usernamebuffer<<std::endl;//testing code
    // Send the target username to the server
    send(clientSocket, usernamebuffer, sizeof(usernamebuffer), 0);
    memset(buffer, 0, sizeof(buffer));
    
    //std::cout<<"\n "<<buffer<<std::endl;
    ////////////////////////////////////get target ip and port from server
    recv(clientSocket,buffer,sizeof(buffer),0);
    sockaddr_in peerAddress;
    // Set the address family and port
    peerAddress.sin_family = AF_INET;
    //peerAddress.sin_port = htons(81);
    std::cout<<" Received otherperson's IP : "<<buffer<<std::endl;//testing code. display ip of otherperson
    int check=0;
    // Convert the IP address from string to binary form
    if (inet_pton(AF_INET, buffer, &(peerAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid IP address\n";
        check=1;
        // Handle error
    }
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket,buffer,sizeof(buffer),0);
    std::cout<<" Received otherperson's Port number : "<<buffer<<std::endl;//testing code. display portnum of otherperson
    //std::cout<<"\n "<<buffer<<std::endl;
    if(check==0){
    int portnum = std::stoi(buffer);
    
    peerAddress.sin_port = htons(portnum);
    //peerAddress.sin_addr.s_addr = INADDR_ANY;///////testing
    // Connect to the peer directly now
    //int check1=3;
    if (connect(peerSocket, (struct sockaddr*)&peerAddress, sizeof(peerAddress)) == -1) {
        	perror("Error connecting to peer");
        	//exit(EXIT_FAILURE);
            return;
    		}  
    send(peerSocket, myname.c_str(), sizeof(myname), 0);//send my target peer my name for helping in cout

    std::cout << "Start chatting (type 'exit' to end):" << std::endl;
            exitRequested=false; //reset flag
            // Handle send in a separate thread
            std::thread T1(sendThreadInitiator, peerSocket, usernamebuffer);
            // Handle receive in a separate thread
            std::thread T2(receiveThreadInitiator, peerSocket, usernamebuffer);

            T1.join();
            T2.join();
    }
    
    
}
void sendThreadTarget(int otherclientSocket, std::string usernamebuffer){
	while(true){   
        if (exitRequested) {
                std::cout << "\nconnection was closed !" << std::endl;
                if(otherclientSocket!=-1){
                    if (close(otherclientSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    otherclientSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                break;
                return;
        }else{
                std::cout << "Enter Message for '" << usernamebuffer << "': " << std::endl;
                std::string message;
                std::getline(std::cin, message);            
                send(otherclientSocket, message.c_str(), message.size(), 0);
                if (message == "exit") {
                    std::cout << "\nYou closed the connection!" << std::endl;
                    exitRequested = true;
                    if(otherclientSocket!=-1){
                        if (close(otherclientSocket) == 0) {
                        // Mark the descriptor as closed by setting it to -1
                        otherclientSocket = -1;
                        std::cout<<" Socket closed "<<std::endl;
                        } else {
                        std::cerr << "Error closing socket. Socket already closed." << std::endl;
                        }
                    }
                    break;
                    return;
                }  
        }
    }
}
void receiveThreadTarget(int otherclientSocket, std::string usernamebuffer){
    char buffer[MAX_BUFFER_SIZE];
	while(true){ 
        if (exitRequested) {
                //std::cout << "\nConnection was closed by yourself '" << usernamebuffer << "'!" << std::endl;
                if(otherclientSocket!=-1){
                    if (close(otherclientSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    otherclientSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                break;
                return;
        }
        else{
            memset(buffer, 0, sizeof(buffer));
            int recbytes = recv(otherclientSocket, buffer, sizeof(buffer), 0);
            std::string str(buffer);

            if (str == "exit") {
                std::cout << "\nConnection was closed by '" << usernamebuffer << "'!" << std::endl;
                std::cout << "Enter 'exit' to close your sending thread too !" << std::endl;
                exitRequested = true;
                send(otherclientSocket, "exitReciprocated", MAX_BUFFER_SIZE, 0);
                //close(otherclientSocket);
                //break;
                //return;
            } 
            else if(str == "exitReciprocated"){
                exitRequested=true;
                break;
                return;
            }
            else if (recbytes <= 0 ) {
                exitRequested = true;
                if(otherclientSocket!=-1){
                    if (close(otherclientSocket) == 0) {
                    // Mark the descriptor as closed by setting it to -1
                    otherclientSocket = -1;
                    std::cout<<" Socket closed "<<std::endl;
                    } else {
                    std::cerr << "Error closing socket. Socket already closed." << std::endl;
                    }
                }
                
                break;
                return;
            }else{
                std::cout << "Received Message from '" << usernamebuffer << "': " << buffer << std::endl;
                std::cout << "Enter Message for '" << usernamebuffer << "': " << std::endl;
            }
        }
    }
}
void acceptConnections(int mySocket, int otherclientSocket, std::string mydirectory){
    char buffer[MAX_BUFFER_SIZE];
	while(true){ 
                //memset(buffer, 0, sizeof(buffer));
        struct sockaddr_in otherclientAddress;
        socklen_t otherclientAddressLen = sizeof(otherclientAddress);
        otherclientSocket = accept(mySocket, (struct sockaddr*)&otherclientAddress, &otherclientAddressLen);
        //std::cout<<" otherclientSocket = "<<otherclientSocket<<std::endl;
        if (otherclientSocket == -1) {
            perror("Error accepting connection");
            //continue;
        }
        memset(buffer, 0, sizeof(buffer));
        // receiving initiator's name for printing in cout statements or receiving signalmessage to send required file to the peer
        recv(otherclientSocket, buffer, sizeof(buffer) - 1, 0);
        if(std::string(buffer)!="sendmefile"){
            std::cout<<" ' "<<buffer<<" ' connected you for chat. Enter 'start' first to activate sending messages :"<<std::endl;
            if(otherclientSocket>0){
                exitRequested=false; //reset flag
                // Handle send in a separate thread
                std::thread T3(sendThreadTarget, otherclientSocket, buffer);
                // Handle receive in a separate thread
                std::thread T4(receiveThreadTarget, otherclientSocket, buffer);
            T3.join();
            T4.join(); 
            break;                                    
            }
        }else if(std::string(buffer)=="sendmefile"){
            memset(buffer, 0, sizeof(buffer));
            // receive file name to send the file to the peer
            recv(otherclientSocket, buffer, sizeof(buffer), 0);
            //std::cout<<buffer<<std::endl;
            //std::string filenamebuffer(buffer);
            //if(buffer[0]=='.'){
                memmove(buffer, buffer+1,strlen(buffer));
               // filenamebuffer = filenamebuffer.substr(1); //remove unwanted dot(.) which appears in the start, after receiving filename
            //}
            //std::cout<<buffer<<std::endl;
            std::string filePath = mydirectory+std::string(buffer);
            std::cout<<filePath<<std::endl;
            // Example: File path to send
            //std::string filePath(buffer);

            // Send the contents of the file to the server
            sendFile(filePath, otherclientSocket);
            std::cout<<" File ' "<<buffer<<" ' sent successfully !"<<std::endl;
            close(otherclientSocket);

        }
        
    }
    
}

int main() {
    int clientSocket,mySocket,otherclientSocket,peerSocket;
    struct sockaddr_in serverAddress,myAddress,otherclientAddress;
    socklen_t otherclientAddressLen = sizeof(otherclientAddress);
    // Create socket for connecting to server
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    // Create socket for letting other to connect
    mySocket = socket(AF_INET, SOCK_STREAM, 0);
    // Create socket for connecting to other peer for chatting
    peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mySocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    // Initialize server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }    
    char buffer[MAX_BUFFER_SIZE];
    //memset(buffer, 0, sizeof(buffer));
    char mynamebuffer[MAX_BUFFER_SIZE];
    // Get user details
    std::vector<std::string> sharedFiles;
    std::cout << "Enter your username: "<<std::endl;
    std::cin >> mynamebuffer;
    send(clientSocket, mynamebuffer, strlen(mynamebuffer), 0);
    memset(buffer, 0, sizeof(buffer));
    std::cout << "Enter your shared directory: ";
    std::cin >> buffer;
    send(clientSocket, buffer, strlen(buffer), 0);
    //memset(buffer, 0, sizeof(buffer));
    // Get the list of shared files
    //std::vector<std::string> sharedFiles;
    for (const auto& entry : std::filesystem::directory_iterator("/home/faizan/Downloads/CNproject/"+std::string(buffer))) {
        sharedFiles.push_back(entry.path().filename().string());
    }

    // Send the list of file names to the server
    sendFileNames(clientSocket, sharedFiles);
    // Get the list of files in the shared directory
    std::string sharedDirectory = "/home/faizan/Downloads/CNproject/"+std::string(buffer)+"/";  // Replace with your shared directory
    //std::string fileNames = getFilesInDirectory(sharedDirectory);

   // std::cout<<fileNames<<std::endl;  //testing code
    // Send the list of file names to the server
    //send(clientSocket, fileNames.c_str(), fileNames.size(), 0);


    // List shared files
    //std::cout << "Enter your shared files separated by commas(,) and also end with a comma(,): ";
   // std::cin.ignore();
    //std::cin.getline(buffer, MAX_BUFFER_SIZE);
   // send(clientSocket, buffer, strlen(buffer), 0);
    memset(buffer, 0, sizeof(buffer));
    // send public IP for future P2P connections
    std::cout << "Enter your Public IP for future P2P connections : ";
    //std::cin.ignore(-1);
    //std::cin.getline(buffer, MAX_BUFFER_SIZE);
    std::cin>>buffer;
    char ipbuffer[MAX_BUFFER_SIZE];
    strcpy(ipbuffer,buffer);
    //send(clientSocket, buffer, strlen(buffer), 0);
    
    inet_aton(buffer, &myAddress.sin_addr);//set my future p2p connection ip
    memset(buffer, 0, sizeof(buffer));
    // send port number for accepting future P2P connections
    std::cout << "Enter the Port Number, on which you will be accepting future P2P connections : ";
    //std::cin.ignore(-3);
   // std::cin.getline(buffer, MAX_BUFFER_SIZE);
   // send(clientSocket, buffer, strlen(buffer), 0);
   std::cin>>buffer;
   char portbuffer[MAX_BUFFER_SIZE];
   strcpy(portbuffer,buffer);
    
    int p2pPort=std::stoi(buffer);
    // Initialize my own address
    myAddress.sin_family = AF_INET;
    //myAddress.sin_addr.s_addr = INADDR_ANY;
    myAddress.sin_port = htons(p2pPort);// set my future p2p connection portnumber

    // Bind socket for accepting connection from peers...while loop to prompt user to enter ip again and again untill they enter a valid ip
    while(1){
        if (bind(mySocket, (struct sockaddr*)&myAddress, sizeof(myAddress)) == -1) {
    	    perror("Error binding socket");
            memset(ipbuffer, 0, sizeof(ipbuffer));
            std::cout << "Please enter again your valid Public IP for future P2P connections : ";
            //std::cin.getline(buffer, MAX_BUFFER_SIZE);   
            std::cin>>ipbuffer;
            inet_aton(ipbuffer, &myAddress.sin_addr);//set my future p2p connection ip
       	    //exit(EXIT_FAILURE);
        } else {
            send(clientSocket, ipbuffer, sizeof(ipbuffer), 0);//send my valid ip to server for future p2p connection
            send(clientSocket, portbuffer, sizeof(portbuffer), 0);//send my valid portnumber to server for future p2p connection

            break;
        }
    }
       
    
// Listen for incoming connections
    if (listen(mySocket, 4) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }
    // Handle main big thread in a separate thread
        std::thread T0(acceptConnections, mySocket, otherclientSocket, sharedDirectory);		   
    
    while (true) {
        std::cout << "====================" << std::endl;
        std::cout << "1. List shared files" << std::endl;
        std::cout << "2. Search for a file" << std::endl;
        std::cout << "3. Download a file" << std::endl;
        std::cout << "4. Chat with a user" << std::endl;
        std::cout << "5. Exit" << std::endl;

        char choice[MAX_BUFFER_SIZE];
        std::cout << "Enter your choice : ";
        std::cin >> choice;
        // Check if the input is a number
    bool isNumber = true;
    for (int i = 0; choice[i] != '\0'; i++) {
        if (!isdigit(choice[i])) {
            isNumber = false;
            break;
        }
    }
    if (isNumber ) {
        // Convert the input to an integer
        int numericChoice = std::stoi(choice);
        std::cout << "You entered a number: " << numericChoice << std::endl;
    
	    send(clientSocket,choice,1,0);
    
	    int num=std::stoi(choice);
        switch (num) {
            case 1:
                // Display shared files
                receiveFilesNames(clientSocket);
                break;
            case 2:
                // Search for a file
                searchFile(clientSocket);
                break;
            case 3:
                // Download a file
                downloadFile(clientSocket, peerSocket, sharedDirectory);
                //resetting peerSocket for initiating/making new connections (after closing of previous connection)
                peerSocket = socket(AF_INET, SOCK_STREAM, 0); 
                break;
            case 4:
                // Chat with a user
                std::cout<<" Following is the list of usernames of all the peers in the network : "<<std::endl;
                receiveUsernames(clientSocket); //receive names of all peers in the network, to connect to anyone of them
                chatWithUser(clientSocket, peerSocket, mynamebuffer);
                //resetting peerSocket for initiating/making new connections (after closing of previous connection)
                peerSocket = socket(AF_INET, SOCK_STREAM, 0); 
                break;
            case 5:
                // Exit the application
                close(clientSocket);
                close(peerSocket);
                close(mySocket);
                return 0;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    
    } else if(std::string (choice)=="start"){
        
        if(T0.joinable()){
            std::cout << " Chat Activated ! You can now send messages. " << choice << std::endl;
            T0.join();
            //creating new accept connection thread after succesfull completion of previous thread.
            T0 = std::thread(acceptConnections, mySocket, otherclientSocket, sharedDirectory);
        }else {
            std::cout << "Invalid choice. Please try again." << std::endl;
        }         

    }else {
        std::cout << "Invalid choice. Please try again." << std::endl;
    }
}
    return 0;
}
