#include "VideNet.h"

namespace videnet
{
	/* ++++++++++++++++++++++++  VideNetRequestBase  +++++++++++++++++++++++ */
	void VideNetRequest::onResponse(const comps::CanMessage& msg)
	{
		if ((msg.frameId == VIDE_NET_RESPONSE) && getHeader() && (checkType(msg.u8[0])))
			if (memcmp((const void*)&msg.u8[1], getHeader(), 3))
				onFinishedInternal(msg);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++   VideNetChangeParamRequest   +++++++++++++++++++ */
	void VideNetChangeParamRequest::onFinishedInternal(const comps::CanMessage& msg)
	{
		if (onChangeParamFinished)
			onChangeParamFinished();
	}

	bool VideNetChangeParamRequest::checkType(uint8_t type)
	{
		return type == VideNetRequestType::WriteAck;
	}

	VideNetChangeParamRequest::VideNetChangeParamRequest() 
		: VideNetRequest()
	{
		msg.frameId = VIDE_NET_REQUEST;
		msg.u64 = 0;
		msg.dataLen = 8;
		msg.u8[0] = VideNetRequestType::Write;
	}

	const comps::CanMessage& VideNetChangeParamRequest::getMessage() const
	{
		if (getHeader())
			memcpy((void*)&msg.u8[1], getHeader(), 3);

		return msg;
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeBoolParamRequest +++++++++++++++++++ */
	VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest(bool value, std::function<void()>&& onFinishedFn)
		: VideNetChangeParamRequest()
	{
		msg.u8[4] = value;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeUint8ParamRequest +++++++++++++++++++ */
	VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn)
		: VideNetChangeParamRequest()
	{
		msg.u8[4] = value;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */
}