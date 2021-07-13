#pragma once
#include <functional>
#include <inttypes.h>
#include  <cstring>
#include "../components/CanService.h"

#define VIDE_NET_REQUEST 0x604
#define VIDE_NET_RESPONSE 0x584

namespace videnet
{
	namespace VideNetRequestType
	{
		enum TYPE
		{
			Write = 0x22,
			WriteAck = 0x60,
			Read = 0x60,
			ReadAck = 0x43
		};
	}

	class VideNetRequest
	{
		protected:
			virtual bool checkType(uint8_t value) { return false; }
			virtual void onFinishedInternal(const comps::CanMessage& msg) = 0;
			virtual const void* getHeader() const { return nullptr; }
		public:
			void onResponse(const comps::CanMessage& msg);
	};

	class VideNetChangeParamRequest : public VideNetRequest
	{
		protected:
			std::function<void()> onChangeParamFinished = nullptr;
			comps::CanMessage msg;

			bool checkType(uint8_t value) override;
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
		const void* getHeader() const override { return "\x10\x10\x011"; }
	};

	class VideNetSetController : public VideNetChangeBoolParamRequest
	{
		using VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest;
		const void* getHeader() const override { return "\x05\x20\x01"; }
	};

	class VideNetSetHeatMode : public VideNetChangeUint8ParamRequest
	{
		using VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest;
		const void* getHeader() const override { return "\x64\x20\x01"; }
	};

}