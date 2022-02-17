
#include "UdpServer.h"
#include <memory>
#define C_ALL(X)

using namespace std;
const string msg = "UdpServer: ok!\n";

int main(int argc, char** argv){

    std::string host(argv[1]);
    int port = std::atoi(argv[2]);



    std::function<void(std::string&)> handlerMsg = [&](std::string& msgToClient) -> void
    {
        std::vector<int> data;
        //msgToClient = "TcpServer: ok!\n";
        std::regex r(R"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)");
        for (std::sregex_iterator i = std::sregex_iterator(msgToClient.begin(), msgToClient.end(), r);
            i != std::sregex_iterator();
            ++i)
        {
            std::smatch m = *i;
            if (m.str().size() >= 10)
                std::cout << "Too large number."<< m.str() <<"Skiped\n";
            else
                data.push_back(std::stoi(m.str()));
        }
        std::sort(data.begin(), data.end(),[](int a, int b){
        return a > b;});

//           for ( auto & i : data ) {
//               msgToClient += i;
//           }

          int tmp  = std::accumulate(data.begin(), data.end(), 0);
          msgToClient += "\n";
          //msgToClient = std::accumulate(data.begin(), data.end() , "" , toStr);
          msgToClient += std::to_string(tmp) + "\n";
          //std::cout<<msgToClient ;

    };
try
{
    std::shared_ptr< UdpServer> udpServer ( new UdpServer(host, port));
    udpServer->setHandleMsg(handlerMsg);

    udpServer->exec();

   // thread t2(&TcpServer::recvData, std::ref(*tcpServer));
    //thread t3(&TcpServer::sendData, std::ref(*tcpServer));

    //t1.join();
    //t2.detach();
    //t3.detach();
}
catch(const std::exception& e)
{
    std::cerr << e.what() << '\n';
}

return 0;
}
