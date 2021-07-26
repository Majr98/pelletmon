#include "VideNet.h"

namespace videnet
{
	/* ++++++++++++++++++++++++  VideNetRequestBase  +++++++++++++++++++++++ */
	VideNetRequest::VideNetRequest()
	{
		rmsg.frameId = VIDE_NET_REQUEST;
		rmsg.u64 = 0;
		rmsg.dataLen = 8;
	}

	bool VideNetRequest::onResponse(const comps::CanMessage& msg)
	{
		if (getHeader() && (checkType(msg.u8[0])))
		{
			if (memcmp((const void*)&msg.u8[1], getHeader(), 3) == 0)
			{
				onFinishedInternal(msg);
				return true;
			}
		}

		return false;
	}

	const comps::CanMessage& VideNetRequest::prepareMessage()
	{
		if (getHeader())
			memcpy((void*)&rmsg.u8[1], getHeader(), 3);

		sendingTime = millis();

		return rmsg;
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++   VideNetChangeParamRequest   +++++++++++++++++++ */
	VideNetChangeParamRequest::VideNetChangeParamRequest()
	{
		rmsg.u8[0] = VideNetRequestType::Write;
	}

	void VideNetChangeParamRequest::onFinishedInternal(const comps::CanMessage& msg)
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
		rmsg.u8[4] = value ? 1 : 0;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeUint8ParamRequest +++++++++++++++++++ */
	VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn)
	{
		rmsg.u8[4] = value;

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
		rmsg.u8[0] = VideNetRequestType::Read;
	}

	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadBoolParamRequest +++++++++++++++++++ */
	VideNetReadBoolParamRequest::VideNetReadBoolParamRequest(std::function<void(bool)>&& onReadBoolFinishedFn)
	{
		if (onReadBoolFinishedFn)
			onReadBoolFinished = std::move(onReadBoolFinishedFn);

	}
	void VideNetReadBoolParamRequest::onFinishedInternal(const comps::CanMessage& msg)
	{
		if (onReadBoolFinished)
			onReadBoolFinished(msg.u8[4] == 1);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadUint8ParamRequest +++++++++++++++++++ */
	VideNetReadUint8ParamRequest::VideNetReadUint8ParamRequest(std::function<void(uint8_t)>&& onReadUint8FinishedFn)
	{
		if (onReadUint8FinishedFn)
			onReadUint8Finished = std::move(onReadUint8FinishedFn);

	}
	void VideNetReadUint8ParamRequest::onFinishedInternal(const comps::CanMessage& msg)
	{
		if (onReadUint8Finished)
			onReadUint8Finished(msg.u8[4]);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetReadUint16ParamRequest +++++++++++++++++++ */
	VideNetReadUint16ParamRequest::VideNetReadUint16ParamRequest(std::function<void(uint16_t)>&& onReadUInt16FinishedFn)
	{
		if (onReadUInt16FinishedFn)
			onReadUInt16Finished = std::move(onReadUInt16FinishedFn);

	}
	void VideNetReadUint16ParamRequest::onFinishedInternal(const comps::CanMessage& msg)
	{
		if (onReadUInt16Finished)
			onReadUInt16Finished(msg.u16[2]);
	}
	/* --------------------------------------------------------------------- */
}