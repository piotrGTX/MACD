#pragma once

class Boot {
public:

	Boot(size_t wallet = 1000);
	~Boot();

private:
	size_t wallet;
	size_t stocks;
};

