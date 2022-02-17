#include "AbstractServer.h"
#include <mutex>
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <regex>
#include <numeric>

class TcpServer : protected AbstractServer
{
private:
	struct linger {
	u_short l_onoff;
	u_short l_linger;
	};

    std::list<Socket> sockets;
    std::vector<std::thread> connections;
    std::string m_host;
    uint32_t m_port;
    std::string msgToClient = "";
    struct sockaddr_in servaddr, cli;
    SocketState m_servState = Disconnected;
    Socket serverSocket;
    bool flag = false;

    HandlerMsg m_handlerMsg;
    //HandlerConnect m_handlerconnect;
    HandlerDisc m_handlerDisc;
    
    std::mutex m;
public:    
    TcpServer (std::string host, uint32_t port);
    virtual void sendData(Socket) override;
    virtual std::string recvData(Socket) override;
    virtual SocketState status() const override;
    virtual void exec() override;
    Socket getSock();
    bool isNewConnection();
    void setConnectThread(std::thread &&t);
    void conversation(Socket);
    virtual void transformData() override;
	virtual uint8_t closeServer() override;
    Socket closeSocket(Socket);
    void detchThreads();
	virtual void setHandleMsg(HandlerMsg) override;
    //virtual void setHandleConnect(HandlerConnect) override;
    virtual void setHandleDisc(HandlerDisc) override;
    virtual ~TcpServer() {
        closeServer();};
};
Socket TcpServer :: getSock() {
    std::unique_lock<std::mutex> lock(m);
    flag = false;
    if(sockets.empty())
        return -1;
    return sockets.back();
}
bool TcpServer :: isNewConnection(){return flag;}
TcpServer ::TcpServer (std::string host, uint32_t port) : m_host(host), m_port(port)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(m_port);

     if ((bind(serverSocket, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    if ((listen(serverSocket, SOMAXCONN)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
    {
        printf("Server listening..\n");
        m_servState = Connected;
}
}

void TcpServer ::exec()
{   
    socklen_t len = sizeof(cli);
    for(;;)
    {
        if (status() == Connected)
        {
        Socket sock = accept(serverSocket, (struct sockaddr *)&cli, &len);
        
        linger m_sLinger;
	m_sLinger.l_onoff = 1;
	m_sLinger.l_linger = 5;//when close signal
         
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(linger));
        sockets.push_back( std::move(sock) );
        if (sock < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
        {
            printf("server accept the client...\n");
            std::unique_lock<std::mutex> lock(m);
            flag = true;
        }
        }
        else
            printf("server doesnt work...\n");
    }   
}
SocketState TcpServer :: status () const{
    //std::shared_lock<std::mutex> lock(m);
    return m_servState;
}
void TcpServer :: sendData(Socket socket){
    std::unique_lock<std::mutex> lock(m);
    send(socket, msgToClient.c_str(), msgToClient.size(), 0);
    msgToClient.clear();
}
std::string TcpServer::recvData(Socket socket){
    char buff[16];
    int answer;
    //std::unique_lock<std::mutex> lock(m);

        answer = recv(socket, buff, 16, 0);

    if (strncmp("exit", buff, 4) != 0){
        if(answer)
        {
            //std::cout<<buff;
            msgToClient = std::string(buff);
        }
    }

         else{
        m_handlerDisc(socket, sockets);
        return "Client Disconnected";
    }
    return msgToClient;
}
uint8_t TcpServer :: closeServer(){
    std::unique_lock<std::mutex> lock(m);
    for(auto &socket : sockets){
        shutdown(socket,SHUT_RDWR);
        close(socket);
    }
    return 0;
}

void TcpServer :: setHandleMsg(HandlerMsg handler){
    m_handlerMsg = handler;
}
void TcpServer :: transformData(){
    m_handlerMsg(msgToClient);
}
//void TcpServer :: setHandleConnect(HandlerConnect handler){
//    m_handlerconnect = handler;
//}
void TcpServer :: setHandleDisc(HandlerDisc handler){
    m_handlerDisc = handler;
}
void TcpServer :: setConnectThread(std::thread &&t){
    connections.push_back(std::move(t));
}
void TcpServer :: detchThreads(){
    for(auto &th : connections){
        th.detach();
    }
}
Socket TcpServer :: closeSocket(Socket socket){

    std::list<Socket>::iterator findIter = std::find(sockets.begin(), sockets.end(), socket);
    return *findIter;
}
void TcpServer :: conversation(Socket socket){
while(true){
        if ( recvData(socket) == "Client Disconnected" )
          {
            std::cout << "Client Disconnected\n";
            return;
          }

        transformData();
        sendData(socket);
    //connections.push_back(std::move(t2));
   }
}
