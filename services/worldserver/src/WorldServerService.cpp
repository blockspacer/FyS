// MIT License
//
// Copyright (c) 2019 Quentin Balland
// Repository : https://github.com/FreeYourSoul/FyS
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//         of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
//         to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//         copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
//         copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//         AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <spdlog/spdlog.h>
#include <zmq_addon.hpp>
#include <flatbuffers/flatbuffers.h>
#include <WSAction_generated.h>
#include <Notifications_generated.h>
#include "WorldServerService.hh"

namespace fys::ws {

WorldServerService::WorldServerService(const WorldServerContext& ctx)
		:_worldServer(ctx)
{
	_connectionHandler.setupConnectionManager(ctx);

}

void
WorldServerService::runServerLoop() noexcept
{
	SPDLOG_INFO("WorldServer loop started");

	while (true) {
		_connectionHandler.pollAndProcessSubMessage(
				[this](zmq::multipart_t&& msg) {
					if (msg.size() < 3 || msg.size() > 4) {
						SPDLOG_ERROR("Received message is ill formatted, should contains 3 to 4 parts but  has {},  "
									 "message is : {}", msg.size(), msg.str());
						return;
					}
					// part0 = idt
					// part1 = token
					// part2 = content
					// part3 = magic internal server
					std::string idt = std::string(static_cast<char*>  (msg.at(0).data()), msg.at(0).size());
					std::string token = std::string(static_cast<char*>(msg.at(1).data()), msg.at(1).size());
					std::optional<std::string> internalMagic;
					if (msg.size() == 4)
						internalMagic = std::string(static_cast<char*>(msg.at(3).data()), msg.at(3).size());

					if (internalMagic && "testing player" == *internalMagic) {
						SPDLOG_DEBUG("<><><<><><><><><><><>We went inside the testing<><><<><><><><><><><>");
					}

					SPDLOG_DEBUG("message received with idt={}, token={},\nmsg={}", idt, token, msg.str());
					processMessage(std::move(idt), std::move(token), msg.at(2), internalMagic);
				});

		_worldServer.executePendingActions(_connectionHandler);
	}
}

void
WorldServerService::processMessage(std::string&& idt,
								   std::string&& token, const zmq::message_t& content, const std::optional<std::string>& internalMagic)
{
	if (internalMagic) {
		// check internal magic to validate that the message is coming from internal server
	}
	else {
		_worldServer.processPlayerInputMessage(std::move(idt), std::move(token),
				fb::GetWSAction(content.data()), _connectionHandler);
	}
}

}
