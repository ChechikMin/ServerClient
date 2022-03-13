#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"Client.h"
#include <algorithm>
#include <future>
std::vector<int> Client::m_digits;

void engine() {
	
	while (counter <= 2018) {
		Client client;
		client.exec();
		std::this_thread::sleep_for(1000ms);
	}
}
void engineErr() {

	while (true) {
		Client client;
		if (client.execError()) break;
		std::this_thread::sleep_for(1000ms);
	}
}

int main() {

	std::vector < std::future<void> > thPool;
	int threadSize = 100;

	for (size_t i = 0; i < threadSize; i++)
	{
		std::future<void> t1 = std::async(std::launch::async, engine);
		//std::thread t1(&Client::exec, std::ref(client1));
		std::this_thread::sleep_for(500ms);
		thPool.push_back(std::move(t1));
	}
	//for (auto &th : thPool)
	//{
	//	th.wait();
	//}
	//thPool.clear();
	for (size_t i = 0; i < threadSize; i++)
	{
		std::future<void> t1 = std::async(std::launch::async, engineErr);
		//std::thread t1(&Client::exec, std::ref(client1));
		std::this_thread::sleep_for(500ms);
		thPool.push_back(std::move(t1));
	}
	//for (auto &th : thPool)
	//{
	//	th.wait();
	//}
	//std::thread t3(&Client::exec, std::ref(client3));
	//std::thread t3(&Client::collectNum, std::ref(client1));
	/*thPool.push_back(std::move(t1));
	thPool.push_back(std::move(t2));
	thPool.push_back(std::move(t3));*/
	//t4.wait();
	/*thPool[0].join();
	thPool[1].join();
	thPool[2].join();*/
	//std::this_thread::sleep_for(5000ms);
	sort(Client::m_digits.begin(), Client::m_digits.end());
	std::cout << Client::m_digits[Client::m_digits.size() / 2] << std::endl;
}

	//client.exec();
