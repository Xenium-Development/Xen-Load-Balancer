#include "LoadBalancer.h"

PoolTable Table;
SOCKET ServerSocket = INVALID_SOCKET;

int PingInterval = 60000;
int ConnectTimeout = 500000;

bool ReadConfig(std::string fileName) {
	nlohmann::json JSON;
	printf("Reading Config File: %s...", fileName.c_str());

	std::ifstream jsonFile(fileName);
	if (!jsonFile.good()) return false;

	jsonFile >> JSON;

	int newPingInterval = JSON["PingInterval"];
	int newConnectTimeout = JSON["ConnectTimeout"];

	PingInterval = newPingInterval * 1000;
	ConnectTimeout = newConnectTimeout * 1000;
	return true;
}

bool ReadIPTable(std::string fileName) {
	nlohmann::json JSON;
	printf("Reading IP Pool Table: %s...", fileName.c_str());
	std::ifstream jsonFile(fileName);
	if (!jsonFile.good()) return false;

	jsonFile >> JSON;

	for (nlohmann::json IP : JSON["IPTable"]) {
		std::string IPAddress = IP["IP"];
		short Port = IP["Port"];
		inet_addr(IPAddress.c_str()) != -1 ? Table.Addresses.push_back(Server(IPAddress, Port)) : (void)printf("Skipped IP. %s is not a valid IP\n", IPAddress.c_str());
	}

	return true;
}

bool SetupSocketListener(SOCKET* Sock, unsigned short PortNum) {
	WSAData wsaData = { 0 };
	sockaddr_in SockInfo = { 0 };

	SockInfo.sin_family = AF_INET;
	SockInfo.sin_addr.s_addr = INADDR_ANY;
	SockInfo.sin_port = htons(PortNum);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

	*Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	ReuseSockets(*Sock);

	if (bind(*Sock, (sockaddr*)&SockInfo, sizeof(sockaddr_in)) != 0) return false;
	if (listen(*Sock, 5) != 0) return false;
	return true;
}

int GetPoolIPAddress() {

	int ServerID = -1;

	for (size_t i = Table.CurrentServer; i < Table.Addresses.size(); i++) {
		if (!Table.Addresses[i].Online) continue;
		ServerID = i;
		break;
	}
	if (ServerID == -1) {
		for (int j = 0; j < Table.CurrentServer; j++) {
			if (!Table.Addresses[j].Online) continue;
			ServerID = j;
			break;
		}
		if (ServerID == -1) {
			Table.CurrentServer = 0;
			return -1;
		}
	}

	Table.CurrentServer = ServerID;
	Table.CurrentServer = (Table.CurrentServer == Table.Addresses.size() ? 0 : Table.CurrentServer + 1);

	return ServerID;
}

void ClientThread(SOCKET sock, int ServerIndex) {
	printf("Client Connected! %i\n", sock);
	char NewServer[6] = { 0 };

	if (ServerIndex == -1)
		printf("[ERROR]: All Servers Are Offline\n");
	else {
		printf("Server Is: %s:%i\n", Table.Addresses[ServerIndex].IPAddress.c_str(), Table.Addresses[ServerIndex].Port);
		*(int*)(NewServer) = inet_addr(Table.Addresses[ServerIndex].IPAddress.c_str());
		*(short*)(NewServer + 0x4) = Table.Addresses[ServerIndex].Port;
	}
	send(sock, NewServer, 6, 0);
	closesocket(sock);
	shutdown(sock, SD_BOTH);
}

void CheckServers() {
	for (;;) {
		printf("[INFO]: Checking Server Pool For Online Servers!\n");
		for (size_t i = 0; i < Table.Addresses.size(); i++) {
			TIMEVAL Timeout;
			Timeout.tv_sec = 0;
			Timeout.tv_usec = ConnectTimeout;
			struct sockaddr_in address;  /* the libc network address data structure */

			SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			address.sin_addr.s_addr = inet_addr(Table.Addresses[i].IPAddress.c_str()); /* assign the address */
			address.sin_port = htons(Table.Addresses[i].Port);            /* translate int2port num */
			address.sin_family = AF_INET;

			//set the socket in non-blocking

			SetNonBlocking(sock);

			if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == false) {
				Table.Addresses[i].Online = false;

				continue;
			}

			// restart the socket mode
			SetBlocking(sock);

			fd_set Write, Err;
			FD_ZERO(&Write);
			FD_ZERO(&Err);
			FD_SET(sock, &Write);
			FD_SET(sock, &Err);

			// check if the socket is ready
			int rc = select(0, NULL, &Write, &Err, &Timeout);

			bool Status = CheckTimeout(sock, &Write, &Timeout);

			if (Table.Addresses[i].Online != Status) {
				if (Table.Addresses[i].Online && !Status) printf("[WARNING]: Server %i is offline!\n", (int)i + 1);
				else printf("[SUCCESS]: Server %i is online!\n", (int)i + 1);
			}
			Table.Addresses[i].Online = Status;
			shutdown(sock, SD_BOTH);
			closesocket(sock);
		}

		Sleep(PingInterval);
	}
}

int main() {
	if (!ReadConfig("config.json")) {
		printf("Failed Config not found - Using Default Values!\nSet Ping Interval To %i Seconds And Connect Timeout To %i milliseconds!\n", PingInterval / 1000, ConnectTimeout / 1000);
	}
	else printf("Success!\nSet Ping Interval To %i Seconds And Connect Timeout To %i milliseconds!\n", PingInterval / 1000, ConnectTimeout / 1000);

	if (!ReadIPTable("pooltable.json")) {
		printf("Failed! File not found!\n");
		return EXIT_FAILURE;
	}
	printf("Success!\nLoaded %i Servers Into The Pool\n", (int)Table.Addresses.size());

	if (!SetupSocketListener(&ServerSocket, 1234)) {
		printf("Failed With WSA Error: %i\n", WSAGetLastError());
	}
	printf("Waiting For Client Connections...\n");

	std::thread(CheckServers).detach();

	while (true) {
		sockaddr_in ClientSockInfo = { 0 };
		SOCKET ClientSocket = accept(ServerSocket, (sockaddr*)&ClientSockInfo, &SockAddrInSize);
		if (ClientSocket != -1) std::thread(ClientThread, ClientSocket, GetPoolIPAddress()).detach();
	}
}