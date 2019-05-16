#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>

#include "JSON.hpp"
#include "CrossCompile.h"

struct Server {
	std::string IPAddress;
	short Port;
	bool Online;

	Server(std::string IPAddress, short Port) {
		this->IPAddress = IPAddress;
		this->Port = Port;
		this->Online = true;
	}
};

struct PoolTable {
	std::vector<Server> Addresses;
	int CurrentServer;
};

struct IPPacket {
	long IP;
	short Port;
};