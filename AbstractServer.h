#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <list>
#include <functional>

typedef uint8_t Socket;

struct HeartBeat{};

    enum SocketState{
        Connected,
        Disconnected,
    };

class AbstractServer
{
protected:
    typedef std::function<void(std::string&)> HandlerMsg;
    //typedef std::function<void(std::string&)> HandlerConnect;
    typedef std::function<void(Socket,std::list<Socket>)> HandlerDisc;

public:

    virtual void sendData(Socket) = 0;
    virtual void transformData() = 0;    
    virtual std::string recvData(Socket) = 0;
    virtual SocketState status() const = 0;
    virtual void exec() = 0;
    virtual uint8_t closeServer() = 0;
    virtual void setHandleMsg(HandlerMsg) = 0;
    //virtual void setHandleConnect(HandlerConnect) = 0;
    virtual void setHandleDisc(HandlerDisc) = 0;
    virtual ~AbstractServer() = default;
};
