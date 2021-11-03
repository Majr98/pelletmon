#pragma once
#include <functional>
#include <inttypes.h>
#include <esp32_can.h>

#define VIDE_NET_PING_DELAY 15000
#define VIDE_NET_PING 0x75C
#define VIDE_NET_REQUEST 0x604
#define VIDE_NET_RESPONSE 0x584

namespace videnet
{
	namespace VideNetRequestType
	{
		enum TYPE
		{
			Change = 0x22,
			ChangeDone = 0x60,
			Read = 0x40
		};
	}

	class VideNetRequest
	{
		protected:
			CAN_FRAME rmsg;
			virtual bool checkType(uint8_t value) { return false; }
			virtual void onFinishedInternal(const CAN_FRAME& msg) {};
			virtual const void* getHeader() const { return nullptr; }

			unsigned int sendingTime = 0;

		public:
			bool onResponse(const CAN_FRAME& msg);
			unsigned int getSendingTime() const { return sendingTime; }
			CAN_FRAME& prepareMessage();
			virtual bool needWaitForReply() const { return true; }

			VideNetRequest();
	};

	class VideNetPing : public VideNetRequest
	{
		public:
			virtual bool needWaitForReply() const { return false; }

			VideNetPing();
	};

	class VideNetChangeParamRequest : public VideNetRequest
	{
		protected:
			std::function<void()> onChangeParamFinished = nullptr;

			bool checkType(uint8_t value) override;
			void onFinishedInternal(const CAN_FRAME& msg) override;

		public:
			virtual bool needWaitForReply() const { return onChangeParamFinished != nullptr; }
			VideNetChangeParamRequest();
	};

	class VideNetChangeBoolParamRequest : public VideNetChangeParamRequest
	{
		public:
			VideNetChangeBoolParamRequest(bool value, std::function<void()>&& onFinishedFn = nullptr);
	};

	class VideNetChangeUint8ParamRequest : public VideNetChangeParamRequest
	{
		public:
			VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn = nullptr);
	};

	class VideNetReadParamRequest : public VideNetRequest
	{
		protected:
			bool checkType(uint8_t value) override;

		public:
			VideNetReadParamRequest();
	};

	class VideNetReadBoolParamRequest : public VideNetReadParamRequest
	{
		protected:
			std::function<void(bool)> onReadBoolFinished = nullptr;
			virtual void onFinishedInternal(const CAN_FRAME& msg) override;

		public:
			VideNetReadBoolParamRequest(std::function<void(bool)>&& onReadBoolFinishedFn);
	};

	class VideNetReadUint8ParamRequest : public VideNetReadParamRequest
	{
		protected:
			std::function<void(uint8_t)> onReadUint8Finished = nullptr;
			virtual void onFinishedInternal(const CAN_FRAME& msg) override;

		public:
			VideNetReadUint8ParamRequest(std::function<void(uint8_t)>&& onReadUint8FinishedFn);
	};

	class VideNetReadUint16ParamRequest : public VideNetReadParamRequest
	{
		protected:
			std::function<void(uint16_t)> onReadUInt16Finished = nullptr;
			virtual void onFinishedInternal(const CAN_FRAME& msg) override;

		public:
			VideNetReadUint16ParamRequest(std::function<void(uint16_t)>&& onReadUInt16FinishedFn);
	};

	class VideNetReadUint32ParamRequest : public VideNetReadParamRequest
	{
		protected:
			std::function<void(uint32_t)> onReadUInt32Finished = nullptr;
			virtual void onFinishedInternal(const CAN_FRAME& msg) override;

		public:
			VideNetReadUint32ParamRequest(std::function<void(uint32_t)>&& onReadUInt32FinishedFn);
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
		const void* getHeader() const override { return "\x10\x10\x01"; }
	};

	class VideNetSetController : public VideNetChangeBoolParamRequest
	{
		using VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest;
		const void* getHeader() const override { return "\x05\x20\x01"; }
	};

	class VideNetGetController : public VideNetReadBoolParamRequest
	{
		using VideNetReadBoolParamRequest::VideNetReadBoolParamRequest;
		const void* getHeader() const override { return "\x05\x20\x01"; }
	};

	class VideNetGetKettleTemp : public VideNetReadUint16ParamRequest
	{
		using VideNetReadUint16ParamRequest::VideNetReadUint16ParamRequest;
		const void* getHeader() const override { return "\x01\x20\x01"; }
	};

	class VideNetGetHotWaterTemp : public VideNetReadUint16ParamRequest
	{
		using VideNetReadUint16ParamRequest::VideNetReadUint16ParamRequest;
		const void* getHeader() const override { return "\x10\x21\x01"; }
	};

	class VideNetSetHeatMode : public VideNetChangeUint8ParamRequest
	{
		using VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest;
		const void* getHeader() const override { return "\x64\x20\x01"; }
	};

	class VideNetGetHeatMode : public VideNetReadUint8ParamRequest
	{
		using VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest;
		const void* getHeader() const override { return "\x64\x20\x01"; }
	};

	class VideNetSetHotWaterMode : public VideNetChangeUint8ParamRequest
	{
		using VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest;
		const void* getHeader() const override { return "\x14\x20\x01"; }
	};

	class VideNetGetHotWaterMode : public VideNetReadUint8ParamRequest
	{
		using VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest;
		const void* getHeader() const override { return "\x14\x20\x01"; }
	};

	class VideNetGetBurnerUsageTotal : public VideNetReadUint32ParamRequest
	{
		using VideNetReadUint32ParamRequest::VideNetReadUint32ParamRequest;
		const void* getHeader() const override { return "\x24\x21\x01"; }
	};

	class VideNetGetBurnerPower : public VideNetReadUint8ParamRequest
	{
		using VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest;
		const void* getHeader() const override { return "\x05\x40\x01"; }
	};

	/*
	namespace BurnerStatus
	{
		static const char* Desc[] =
		{
			"Standby",
			"Ignition",
			"Heating",
			"Degiklis1",
			"Degiklis2",
			"Modulation",
			"Extinguishing",
			"Cleaning",
			"Working",
			"Stopped"
		};
	}
	*/


	class VideNetGetBurnerStatus : public VideNetReadUint8ParamRequest
	{
		using VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest;	
		const void* getHeader() const override { return "\x07\x20\x01"; }
	};

	class VideNetGetBurnerFailures : public VideNetReadUint16ParamRequest
	{
		using VideNetReadUint16ParamRequest::VideNetReadUint16ParamRequest;
		const void* getHeader() const override { return "\x88\x20\x01"; }
	};
}