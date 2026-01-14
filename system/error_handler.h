#pragma once


#define EXIT_WITH_MESSAGE(content) \
	printf_s(content); \
	std::cin.get(); \
    std::exit(EXIT_FAILURE); \
