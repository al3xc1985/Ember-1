/*
 * Copyright (c) 2015 Ember
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <game_protocol/Packet.h>
#include <game_protocol/ResultCodes.h>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <cstddef>

namespace ember { namespace protocol {

namespace be = boost::endian;

class SMSG_AUTH_RESPONSE final : public Packet {
	State state_ = State::INITIAL;

public:
	ResultCode result;
	std::uint32_t queue_position;
	std::uint32_t billing_time;
	std::uint8_t billing_flags;
	std::uint32_t billing_rested;

	State read_from_stream(spark::SafeBinaryStream& stream) override try {
		BOOST_ASSERT_MSG(state_ != State::DONE, "Packet already complete - check your logic!");

		stream >> result;

		if(result == ResultCode::AUTH_WAIT_QUEUE) {
			stream >> queue_position;
		}

		if(result == ResultCode::AUTH_OK) {
			stream >> billing_time;
			stream >> billing_flags;
			stream >> billing_rested;
		}

		be::little_to_native_inplace(result);
		be::little_to_native_inplace(queue_position);
		be::little_to_native_inplace(billing_time);
		be::native_to_little_inplace(billing_flags);
		be::native_to_little_inplace(billing_rested);

		return (state_ = State::DONE);
	} catch(spark::buffer_underrun&) {
		return State::ERRORED;
	}

	void write_to_stream(spark::SafeBinaryStream& stream) const override {
		stream << be::native_to_little(result);

		if(result == ResultCode::AUTH_WAIT_QUEUE) {
			stream << be::native_to_little(queue_position);
		}

		if(result == ResultCode::AUTH_OK) {
			stream << be::native_to_little(billing_time);
			stream << be::native_to_little(billing_flags);
			stream << be::native_to_little(billing_rested);
		}
	}
};

}} // protocol, ember