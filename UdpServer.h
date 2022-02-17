#include "AbstractServer.h"
#include <iostream>
#include <regex>
#include <mutex>
#include <numeric>
#define NIX(exp) exp
#define WIN(exp)

class UdpServer : protected AbstractServer
{
private:
    std::list<Socket> sockets;
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
    UdpServer (std::string host, uint32_t port);
    virtual void sendData(Socket) override;
    virtual std::string recvData(Socket) override;
    virtual SocketState status() const override;
    virtual void exec() override;
    virtual void transformData() override;
    virtual uint8_t closeServer() override;
    virtual void setHandleMsg(HandlerMsg) override;
    //virtual void setHandleConnect(HandlerConnect) override;
    virtual void setHandleDisc(HandlerDisc) override;
    virtual ~UdpServer() {
        closeServer();};
};

//bool UdpServer :: isNewConnection(){return flag;}
UdpServer ::UdpServer (std::string host, uint32_t port) : m_host(host), m_port(port)
{
    serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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

      	m_servState = Connected;

}

void UdpServer ::exec()
{   
    socklen_t len = sizeof(cli);
    for(;;)
    {
        if (status() == Connected)
        {
            recvData(serverSocket);
	    transformData();                      
            sendData(serverSocket);              
         }

     }   
}
SocketState UdpServer :: status () const{
    //std::shared_lock<std::mutex> lock(m);
    return m_servState;
}
void UdpServer :: sendData(Socket socket){
   
    socklen_t len = sizeof(cli);
    sendto(socket, msgToClient.c_str(), strlen(msgToClient.c_str()), 
        MSG_CONFIRM, (const struct sockaddr *) &cli,
            len);
    msgToClient.clear();
}
std::string UdpServer::recvData(Socket socket){
    char buff[16];
    int answer;
    	socklen_t len = sizeof(cli);

        answer = recvfrom(socket, buff, 16, MSG_WAITALL, ( struct sockaddr *) &cli,
                &len);

        if(answer)
        {
            //std::cout<<buff;
            msgToClient = std::string(buff);
        }
    return msgToClient;
}
uint8_t UdpServer :: closeServer(){
        close(serverSocket);
    return 0;
    }
    
void UdpServer :: setHandleDisc(HandlerDisc handler){
    m_handlerDisc = handler;
}
void UdpServer :: setHandleMsg(HandlerMsg handler){
    m_handlerMsg = handler;
}
void UdpServer :: transformData(){
    m_handlerMsg(msgToClient);

}

