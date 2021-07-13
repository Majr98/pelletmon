#include "VideNet.h"

namespace videnet
{
	/* ++++++++++++++++++++++++  VideNetRequestBase  +++++++++++++++++++++++ */
	void VideNetRequest::onResponse(const comps::CanMessage& msg)
	{
		if ((msg.frameId == 0x584) && (msg.u8[0] == requestType))
		{
			if (memcmp(&msg.u8[1], getHeader(), 3))
			{
				onFinishedInternal(msg);
			}
		}
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++   VideNetChangeParamRequest   +++++++++++++++++++ */
	void VideNetChangeParamRequest::onFinishedInternal(const comps::CanMessage& msg)
	{
		if (onChangeParamFinished)
			onChangeParamFinished();
	}

	VideNetChangeParamRequest::VideNetChangeParamRequest()
	{
		msg.frameId = 0x584;
		msg.u64 = 0;

		requestType = VideNetRequestType::Write;
		msg.u8[0] = requestType;
		memcpy(&msg.u8[1], getHeader(), 3);
	}

	const comps::CanMessage& VideNetChangeParamRequest::getMessage() const
	{
		return msg;
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeBoolParamRequest +++++++++++++++++++ */
	VideNetChangeBoolParamRequest::VideNetChangeBoolParamRequest(bool value, std::function<void()>&& onFinishedFn)
	{
		msg.u8[4] = value;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */

	/* +++++++++++++++++++ VideNetChangeUint8ParamRequest +++++++++++++++++++ */
	VideNetChangeUint8ParamRequest::VideNetChangeUint8ParamRequest(uint8_t value, std::function<void()>&& onFinishedFn)
	{
		msg.u8[4] = value;

		if (onFinishedFn)
			onChangeParamFinished = std::move(onFinishedFn);
	}
	/* --------------------------------------------------------------------- */
}