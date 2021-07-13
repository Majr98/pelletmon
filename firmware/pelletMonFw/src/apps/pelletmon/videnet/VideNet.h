#pragma once
#include <functional>
#include <inttypes.h>
#include  <cstring>
#include "../components/CanService.h"

namespace videnet
{
	namespace VideNetRequestType
	{
		enum TYPE
		{
			Write = 0x22,
			Read = 0x60
		};
	}

	class VideNetRequest
	{
		protected:
			VideNetRequestType::TYPE requestType;
			virtual const void* getHeader() { return nullptr; }
			virtual void onFinishedInternal(const comps::CanMessage& msg) = 0;

		public:
			void onResponse(const comps::CanMessage& msg);
	};

	class VideNetChangeParamRequest : public VideNetRequest
	{
		protected:
			std::function<void()> onChangeParamFinished = nullptr;
			comps::CanMessage msg;

			void onFinishedInternal(const comps::CanMessage& msg) override;

		public:
			explicit VideNetChangeParamRequest();
			const comps::CanMessage& getMessage() const;
	};

	class VideNetChangeBoolParamRequest : public VideNetChangeParamRequest
	{
		public:
			explicit VideNetChangeBoolParamRequest(bool value, std::function<void()>&& onFinishedFn = nullptr);
	};

	class VideNetChangeUint8ParamRequest : public VideNetChangeParamRequest
	{
		public:
			explicit VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn = nullptr);
	};

	namespace HeatMode
	{
		enum TYPE
		{
			Off,
			Eco,
			Timer,
			Comfort
		};
	}

	class VideNetSaveSettings : public VideNetChangeParamRequest
	{
		using VideNetChangeParamRequest::VideNetChangeParamRequest;
		const void* getHeader() override { return "\x10\x10\x011"; }
	};

	class VideNetSetController : public VideNetChangeBoolParamRequest
	{
		using VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest;
		const void* getHeader() override { return "\x05\x20\x01"; }
	};

	class VideNetSetHeatMode : public VideNetChangeUint8ParamRequest
	{
		using VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest;
		const void* getHeader() override { return "\x64\x20\x01"; }
	};

}