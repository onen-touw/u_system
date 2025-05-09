#pragma once

#include "config.h"
#include "error_helper.h"
#include <stdarg.h>

#define UFO_TERMINAL_MSG_IN "\033[1;"
#define UFO_TERMINAL_MSG_OUT "\033[0m"

#define UFO_TERMINAL_FONT_COLOR_RED "31m"
#define UFO_TERMINAL_FONT_COLOR_BLUE "34m"
#define UFO_TERMINAL_FONT_COLOR_YELLOW "33m"
#define UFO_TERMINAL_FONT_COLOR_WHITE "37m"
// #define UFO_TERMINAL_FONT_COLOR_WHITE ""

#ifdef UFO_ERROR_TRACE_BG_COLORED
#	define UFO_TERMINAL_BACKGROUNF_COLOR_RED "41;"
#	define UFO_TERMINAL_BACKGROUNF_COLOR_BLUE "44;"
#	define UFO_TERMINAL_BACKGROUNF_COLOR_YELLOW "43;"
#	define UFO_TERMINAL_BACKGROUNF_COLOR_WHITE "47;"
#endif

namespace ufo {
	class Trace_t
	{
		friend Error_t;
	private:
		// TODO methods for log

	public:
		Trace_t(/* args */) {
			
		}
		~Trace_t() {}

		static void log(const char* msg) {
			// std::cout << msg;
			printf(msg);
		}

		static void log(char c) {
			printf("%c", c);
			// std::cout << c;
		}
		static void log(unsigned char c) {
			printf("%u", c);
			// std::cout << c;
		}
		
		static void log(int i) {
			printf("%d", i);
			// std::cout << i;
		}
		static void log(unsigned int i) {
			printf("%u", i);
			// std::cout << i;
		}

		static void log(long i) {
			printf("%ld", i);
			// std::cout << i;
		}
		static void log(unsigned long i) {
			printf("%lu", i);
			// std::cout << i;
		}

		static void log(long long i) {
			printf("%lld", i);
			// std::cout << i;
		}
		static void log(unsigned long long i) {
			printf("%llu", i);
			// std::cout << i;
		}

		static void log(float i) {
			printf("%f", i);
			// std::cout << i;
		}

		static void flog(const char* msg, ...){
			va_list ap;
			va_start(ap, msg);
			vprintf(msg, ap);
			va_end(ap);
		}

		static void log(CriticalError_t& e)
		{
			printf(UFO_TERMINAL_MSG_IN);

			// std::cout << UFO_TERMINAL_MSG_IN
			#ifdef UFO_ERROR_TRACE_BG_COLORED
			printf(UFO_TERMINAL_BACKGROUNF_COLOR_RED);
			printf(UFO_TERMINAL_FONT_COLOR_WHITE);

				// << UFO_TERMINAL_BACKGROUNF_COLOR_RED
				// << UFO_TERMINAL_FONT_COLOR_WHITE
			#else
			printf(UFO_TERMINAL_FONT_COLOR_RED);
			
				// << UFO_TERMINAL_FONT_COLOR_RED
			#endif
			printf("Critical error: ");
				// << ;
			log(e._info);
			
			printf(UFO_TERMINAL_MSG_OUT "\n");
			// std::cout << UFO_TERMINAL_MSG_OUT
				// << "\n";
		}
		static void log(CountingError_t& e)
		{
			// std::cout << UFO_TERMINAL_MSG_IN
			#ifdef UFO_ERROR_TRACE_BG_COLORED
			printf(UFO_TERMINAL_BACKGROUNF_COLOR_RED);
			printf(UFO_TERMINAL_FONT_COLOR_WHITE);

				// << UFO_TERMINAL_BACKGROUNF_COLOR_RED
				// << UFO_TERMINAL_FONT_COLOR_WHITE
			#else
			printf(UFO_TERMINAL_FONT_COLOR_BLUE);
			
				// << UFO_TERMINAL_FONT_COLOR_RED
			#endif
			printf("Counting error is reached: %d", static_cast<int>(e.GetCount()));
			
				// << "Counting error is reached "
				// << static_cast<int>(e.GetCount()) << " ";
			log(e._info);
			printf(UFO_TERMINAL_MSG_OUT "\n");

			// std::cout << UFO_TERMINAL_MSG_OUT
			// 	<< "\n";
		}
		static void log(Warning_t& e)
		{
	// std::cout << UFO_TERMINAL_MSG_IN
	#ifdef UFO_ERROR_TRACE_BG_COLORED
	printf(UFO_TERMINAL_BACKGROUNF_COLOR_RED);
	printf(UFO_TERMINAL_FONT_COLOR_WHITE);

		// << UFO_TERMINAL_BACKGROUNF_COLOR_RED
		// << UFO_TERMINAL_FONT_COLOR_WHITE
	#else
	printf(UFO_TERMINAL_FONT_COLOR_YELLOW);
	
		// << UFO_TERMINAL_FONT_COLOR_RED
	#endif
	
				// << "Warning: ";
			log(e._info);

			printf(UFO_TERMINAL_MSG_OUT "\n");

			// std::cout << UFO_TERMINAL_MSG_OUT
				// << "\n";
		}

		// common log for error
		static void log(error::ErrorMinimal_t& i)
		{

			// std::cout
#ifdef UFO_ERROR_USE_TIMESTAMP
				printf("[%d] ", i._time);

				// << "[" << i._time << "] "
#endif
#ifdef UFO_ERROR_USE_TAG
				printf("[%d] ", i._tag);

				// << "[" 
				// << (i._tag ? i._tag : "NTG")
				// << "] - "
#endif
				printf("In file [%s] Line [%d] Code [%d]\nMsg:\n\t%s", (i._place ? i._place : "?"), i._line, i._code, (i._msg ? i._msg : "NON"));

				// << "In file ["
				// << (i._place ? i._place : "?")
				// << "] Line ["
				// << i._line
				// << "] Code ["
				// << i._code
				// << "]\nMsg:\n\t"
				// << (i._msg ? i._msg : "NON");
		}
	};

}   // ufo
