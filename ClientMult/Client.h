#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <condition_variable>
#include <winsock2.h>
#include <mstcpip.h> 
#include <fstream>
#include <thread>
#include<numeric>
#include<regex>
#include <mutex>
#include <algorithm>
#include <vector>
#include <atomic>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <stdio.h>

std::atomic<size_t> counter = 1;
std::vector<size_t> ex;



// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


using namespace std;

class Client
{
public:
	Client();
	void exec();
	bool execError();
	void collectNum();//handle
	static std::vector<int> m_digits;
	~Client();
public:
	bool receive();
	bool sendMsg();
	bool sendMsgErr();
private:
	bool reconnect();

private:
	int m_i = 0;

	std::atomic <SOCKET> m_ConnectSocket = INVALID_SOCKET;
	std::atomic<bool> m_flag = false;
	
	std::string digit;
	
	std::string m_strWithDig = " ";
	std::mutex              g_lockWrite;
	std::condition_variable g_queuecheck;
	std::condition_variable g_Send;
	WSADATA m_wsaData;
	SOCKADDR_IN addr;
	size_t sizeBuff = 64;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	ofstream file;
};

Client::Client()
{
	std::unique_lock<std::mutex> locker(g_lockWrite);
	WSAStartup(MAKEWORD(2, 0), &m_wsaData);
	if ((m_ConnectSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
	}

	addr.sin_addr.s_addr = inet_addr("88.212.241.115"); //коннект к серверу
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2013); //порт
	if (connect(m_ConnectSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		cout << "Server connection failed with error: " << WSAGetLastError() << endl;
	}
	struct timeval time_val_struct;
	time_val_struct.tv_sec = 1;
	time_val_struct.tv_usec = 500000;
	int return_value = setsockopt(m_ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_val_struct, sizeof(time_val_struct));
	if (return_value == -1)
		cout << "setSocketopt Recv failed!" << endl;
	int keepalive = 1;
	//setsockopt(m_ConnectSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keepalive, sizeof(keepalive));


	//struct tcp_keepalive kavars[1] = { 1,        0.5 * 1000,        /* 10 seconds */        0.5 * 1000          /* 5 seconds */ }; 

	//
	// Send socket IO code.
	//
   //if (setsockopt        (m_ConnectSocket, SOL_SOCKET, SO_KEEPALIVE, (const char *) &keepalive,sizeof keepalive) != 0)
   //{        printf ("Set keep alive error: %s.\n",strerror_s (buff, errno));  }  
   //if (WSAIoctl        (m_ConnectSocket, SIO_KEEPALIVE_VALS, kavars, sizeof kavars, NULL, sizeof (int), reinterpret_cast<DWORD*>(&ret), NULL, NULL) != 0)
   //{
	  // printf("Set keep alive error: %s.\n", strerror_s (buff, WSAGetLastError ()));     
   //}
	int keepInterval = 5;
	setsockopt(m_ConnectSocket, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&keepInterval, sizeof(keepInterval));

	struct tcp_keepalive alive;
	DWORD dwRet, dwSize;

	alive.onoff = 1;
	alive.keepalivetime = 1000;
	alive.keepaliveinterval = 100;

	dwRet = WSAIoctl(m_ConnectSocket, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
		NULL, 0, reinterpret_cast<DWORD*>(&dwSize), NULL, NULL);
	if (dwRet == SOCKET_ERROR)
	{
		std::cerr << "WSAIoctl fail with code " << WSAGetLastError() << "\n";
	}


   int nNetTimeout = 9000;
   int nNetTimeout1 = 1000;

   setsockopt(m_ConnectSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout1, sizeof(int));

   setsockopt(m_ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
   
   struct linger {
	   u_short l_onoff;
	   u_short l_linger;
   };

   linger m_sLinger;

   // (Вызывается в close (), но данные остаются, когда они не отправляются)
   // Если m_sLinger.l_onoff = 0; функция такая же, как 2)
   m_sLinger.l_onoff = 1;

   // (Допустимое время пребывания 5 секунд)
   m_sLinger.l_linger = 5;

   setsockopt(m_ConnectSocket, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(linger));
/*
	LPDWORD lpcbBytesReturned;
	INT nSize;

	WSAIoctl(m_ConnectSocket,
		SIO_KEEPALIVE_VALS,
		(LPVOID)&tcpkeepalive,
		sizeof(tcpkeepalive),
		NULL, 0,
		reinterpret_cast<DWORD*>(&nSize),
		NULL, NULL);*/

	cout << "Connected to server!" << endl;
	file.open("Data.txt", ios_base::app);
	if (file.is_open())
	{
		cout << "FileOpen!" << endl;
	}
	else 
	{
		cout << "NeedReopen!" << endl;
	}


}
bool Client::receive()
{
	while (true) {
		std::string buff(3, ' ');
		//while (buff != "\n")
		//{
		int len = recv(m_ConnectSocket, (char*)buff.c_str(), buff.size(), 0);
			if (len == SOCKET_ERROR) {
				cout << "recv function failed with error: " << WSAGetLastError() << endl;
				{
					std::unique_lock<std::mutex> locker(g_lockWrite);
					digit = m_strWithDig;
				}
				//g_queuecheck.notify_all();
				std::this_thread::sleep_for(500ms);
				m_strWithDig = " ";
				{
					std::unique_lock<std::mutex> locker(g_lockWrite);
					ex.push_back(m_i);

				}
				cout << "Erorr: " << WSAGetLastError() << endl;
				return false;
			}
			if (len == 0)
			{
					cout << "Empty msg: " << WSAGetLastError() << endl;
					{
						std::unique_lock<std::mutex> locker(g_lockWrite);
						ex.push_back(m_i);

					}
						return false;
			}
					
				
			m_strWithDig += buff;
				

			cout << "Server: " << buff.c_str() << endl;

			if (buff.substr(0,1) ==  "\n" )
			{
				{
					std::unique_lock<std::mutex> locker(g_lockWrite);
					digit = m_strWithDig;
				}

				//g_queuecheck.notify_all();
				//g_Send.notify_all();
				std::this_thread::sleep_for(1000ms);
				m_strWithDig = " ";
				return true;
			}
			
	}
	return false;

}

bool Client::sendMsgErr() {
	//std::string s;
	//while (true) {
	if (ex.empty())
		return true;
	{

		std::unique_lock<std::mutex> locker(g_lockWrite);
		if(ex[0] == 0)
			return true;
		string s = std::to_string(ex[0]) + "\n";
		ex.erase(ex.begin());
		if (send(m_ConnectSocket, (char*)s.data(), s.size(), 0) == SOCKET_ERROR) {
			cout << "send failed with error: " << WSAGetLastError() << endl;
			return true;

		}
		cout << "Sended Err: " << s << endl;

	}
	counter++;

	return false;
}

bool Client::sendMsg()
{

	m_i = counter;
	string s = std::to_string(m_i) + "\n";
		if (send(m_ConnectSocket, (char*)s.data(), s.size(), 0) == SOCKET_ERROR) {
			cout << "send failed with error: " << WSAGetLastError() << endl;
			return true;

			
		}
		cout << "Sended: " << m_i << endl;
		counter++;
		return false;

}

void Client::collectNum()
{

		if (digit.size() > 2) {
			digit.erase(std::remove(digit.begin(), digit.end(), ' '), digit.end());
			std::cout << "Collecter: " << digit << endl;

			std::string tmp = digit;
			std::regex r(R"([+]?\d*?\d+(?:[eE][-+]?\d+)?)");
			for (std::sregex_iterator i = std::sregex_iterator(tmp.begin(), tmp.end(), r);
				i != std::sregex_iterator();
				++i)
			{
				std::smatch m = *i;
				if (m.str().size() >= 10) {
					std::cout << "Too large number." << m.str() << "Skiped\n";
				}
				else if (m.str().size() <= 10)
				{
					std::unique_lock<std::mutex> locker(g_lockWrite);
					m_digits.push_back(std::stoi(m.str()));
					file << std::stoi(m.str()) << "\n";
					std::cout << "Collecter:" << m_digits.size() << endl;
					digit = " ";
				}
			}
	
	
		}
}

void Client::exec() 
{
	while (true){
		
		if (counter >= 2018) break;
		if (sendMsg()) return;
		if (!receive()) return;
		collectNum();
	}
}

bool Client::execError()
{
	while (true) {
		{
			std::unique_lock<std::mutex> locker(g_lockWrite);
			if (m_digits.size() >= 2018) break;
		}
		if (sendMsgErr()) return true;
		if (!receive())return true;
		collectNum();
	}
	return true;
}

Client::~Client()
{
	std::unique_lock<std::mutex> locker(g_lockWrite);
	file.close();
	closesocket(m_ConnectSocket);
	WSACleanup();
}