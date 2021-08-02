#include "VideNet.h"
#include <Arduino.h>

namespace videnet
{
	/* ++++++++++++++++++++++++  VideNetRequestBase  +++++++++++++++++++++++ */
	VideNetRequest::VideNetRequest()
	{
		memset(&rmsg, 0, sizeof(CAN_frame_t));

		rmsg.MsgID = VIDE_NET_REQUEST;
		rmsg.FIR.B.DLC = 8;
	}

	bool VideNetRequest::onResponse(const CAN_frame_t& msg)
	{
		if (getHeader() && (checkType(msg.data.u8[0])))
		{
			if (memcmp((const void*)&msg.data.u8[1], getHeader(), 3) == 0)
			{
				onFinishedInternal(msg);
				return true;
			}
		}

		return false;
	}

	const CAN_frame_t& VideNetRequest::prepareMessage()
	{
		if (getHeader())
			memcpy((void*)&rmsg.data.u8[1], getHeader(), 3);

		sendingTime = millis();

		return rmsg;
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++   VideNetChangeParamRequest   +++++++++++++++++++ */
	VideNetChangeParamRequest::VideNetChangeParamRequest()
	{
		rmsg.data.u8[0] = VideNetRequestType::Write;
	}

	void VideNetChangeParamRequest::onFinishedInternal(const CAN_frame_t& msg)
	{
		if (onChangeParamFinished)
			onChangeParamFinished();
	}

	bool VideNetChangeParamRequest::checkType(uint8_t type)
	{
		return type == VideNetRequestType::WriteDone;
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeBoolParamRequest +++++++++++++++++++ */
	VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest(bool value, std::function<void()>&& onFinishedFn)
	{
		rmsg.data.u8[4] = value ? 1 : 0;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeUint8ParamRequest +++++++++++++++++++ */
	VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn)
	{
		rmsg.data.u8[4] = value;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */

	/* ------------------------------------------------------------------------------------------------------------------------------------------ */

	/* +++++++++++++++++++   VideNetReadParamRequest   +++++++++++++++++++ */
	bool VideNetReadParamRequest::checkType(uint8_t type)
	{
		return type == VideNetRequestType::ReadDone;
	}

	VideNetReadParamRequest::VideNetReadParamRequest()
	{
		rmsg.data.u8[0] = VideNetRequestType::Read;
	}

	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadBoolParamRequest +++++++++++++++++++ */
	VideNetReadBoolParamRequest::VideNetReadBoolParamRequest(std::function<void(bool)>&& onReadBoolFinishedFn)
	{
		if (onReadBoolFinishedFn)
			onReadBoolFinished = std::move(onReadBoolFinishedFn);

	}
	void VideNetReadBoolParamRequest::onFinishedInternal(const CAN_frame_t& msg)
	{
		if (onReadBoolFinished)
			onReadBoolFinished(msg.data.u8[4] == 1);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadUint8ParamRequest +++++++++++++++++++ */
	VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest(std::function<void(uint8_t)>&& onReadUint8FinishedFn)
	{
		if (onReadUint8FinishedFn)
			onReadUint8Finished = std::move(onReadUint8FinishedFn);

	}
	void VideNetReadUint8ParamRequest::onFinishedInternal(const CAN_frame_t& msg)
	{
		if (onReadUint8Finished)
			onReadUint8Finished(msg.data.u8[4]);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadUint16ParamRequest +++++++++++++++++++ */
	VideNetReadUint16ParamRequest::VideNetReadUint16ParamRequest(std::function<void(uint16_t)>&& onReadUInt16FinishedFn)
	{
		if (onReadUInt16FinishedFn)
			onReadUInt16Finished = std::move(onReadUInt16FinishedFn);

	}
	void VideNetReadUint16ParamRequest::onFinishedInternal(const CAN_frame_t& msg)
	{
		if (onReadUInt16Finished)
			onReadUInt16Finished(msg.data.u8[5] << 8 | msg.data.u8[4]);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadUint32ParamRequest +++++++++++++++++++ */
	VideNetReadUint32ParamRequest::VideNetReadUint32ParamRequest(std::function<void(uint32_t)>&& onReadUInt32FinishedFn)
	{
		if (onReadUInt32FinishedFn)
			onReadUInt32Finished = std::move(onReadUInt32FinishedFn);

	}
	void VideNetReadUint32ParamRequest::onFinishedInternal(const CAN_frame_t& msg)
	{
		if (onReadUInt32Finished)
			onReadUInt32Finished(msg.data.u32[1]);
	}
	/* --------------------------------------------------------------------- */
	
	/* +++++++++++++++++++++++++++++ VideNetPing +++++++++++++++++++++++++++ */
	VideNetPing::VideNetPing()
	{
		rmsg.MsgID = VIDE_NET_PING;
		rmsg.FIR.B.DLC = 1;
		rmsg.data.u8[0] = 1;
	}
	/* --------------------------------------------------------------------- */
}