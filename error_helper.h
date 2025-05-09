#pragma once

#include "config.h"
#include "utils.h"
#include "error_types.h"

#ifdef UFO_ERROR_USE_TIMESTAMP

#ifdef UFO_ERROR_USE_TAG
#define GenerateInfo(tag, code, msg) (ufo::error::ErrorMinimal_t {millis(), tag ,__LINE__, code, __FILE__, msg})		
#define GenerateCoreInfo(msg) (ufo::error::ErrorMinimal_t {millis(), "CORE" ,__LINE__, UFO_CORE_ERROR_CODE, __FILE__, msg})		
#else
#define GenerateInfo(code, msg) (ufo::error::ErrorMinimal_t {millis() ,__LINE__, code, __FILE__, msg})		
#define GenerateCoreInfo(msg) (ufo::error::ErrorMinimal_t {millis(),__LINE__, UFO_CORE_ERROR_CODE, __FILE__, msg})		
#endif // UFO_ERROR_USE_TAG

#else

#ifdef UFO_ERROR_USE_TAG
#define GenerateInfo(tag, code, msg) (ufo::error::ErrorMinimal_t { tag ,__LINE__, code, __FILE__, msg })		
#define GenerateCoreInfo(msg) (ufo::error::ErrorMinimal_t {"CORE",__LINE__, UFO_CORE_ERROR_CODE, __FILE__, msg})		
#else
#define GenerateInfo(code, msg) (ufo::error::ErrorMinimal_t {__LINE__, code, __FILE__, msg})		// todo tag
#define GenerateInfo_Code(code, msg) (ufo::error::ErrorMinimal_t {__LINE__, static_cast<uint16_t>(code), __FILE__, msg})		// todo tag
#define GenerateCoreInfo(msg) (ufo::error::ErrorMinimal_t {\
	__LINE__, \
	static_cast<uint16_t>(ufo::error::codes_t::internal_generic), \
	__FILE__, \
	msg \
	})		
#endif

#endif // UFO_ERROR_USE_TIMESTAMP


namespace ufo {

		namespace error {
			struct ErrorMinimal_t
			{
#ifdef UFO_ERROR_USE_TIMESTAMP
				time_t _time = 0;
#endif
#ifdef UFO_ERROR_USE_TAG
				const char* _tag = nullptr;
#endif
				uint16_t _line = 0;
				uint16_t _code = 0;
				const char* _place = nullptr;
				const char* _msg = nullptr;
			};

			using CriticalErrorHelper_t = ErrorMinimal_t;
			using WarningHelper_t = ErrorMinimal_t;

		} // error

		struct CriticalError_t
		{
			error::CriticalErrorHelper_t _info;
			explicit CriticalError_t() {}
			explicit CriticalError_t(error::ErrorMinimal_t info) : _info(info){}
		};

		struct CountingError_t {
			friend Error_t;
		protected:
			uint8_t _cnt = 0;
		public:
			error::ErrorMinimal_t _info;
			CountingError_t(){}
			explicit CountingError_t(error::ErrorMinimal_t info) : _info(info){}

			uint8_t GetCount() const { return _cnt; }
		};

		struct Warning_t
		{
			error::WarningHelper_t _info;
			explicit Warning_t() {}
			explicit Warning_t(error::ErrorMinimal_t info) : _info(info){}
		};

		enum class WarningLevel_t
		{
			W_ALL,      // Warnings, critical and counting when it reach limit are error
			W_1,        // critical and counting when it reach limit are errors
			W_2         // only critical  is error and counting when it reach limit is warning
		};

} // ufo