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
#include <functional>
#include <zmq_addon.hpp>

#include <flatbuffers/flatbuffers.h>
#include <FightingPitEncounter_generated.h>
#include <ArenaServerAuth_generated.h>
#include <ArenaServerValidateAuth_generated.h>

#include <network/DBConnector.hh>
#include <fightingPit/FightingPitAnnouncer.hh>

#include <ArenaServerContext.hh>
#include "ArenaServerService.hh"

// anonymous namespace used for utility function to extract data from flatbuffer
namespace {

fys::arena::FightingPit::Level
translateLevelFromFlatbuffer(const fys::fb::Level& level)
{
	switch (level) {
	case fys::fb::Level_EASY:return fys::arena::FightingPit::EASY;
	case fys::fb::Level_MEDIUM:return fys::arena::FightingPit::MEDIUM;
	case fys::fb::Level_HARD:return fys::arena::FightingPit::HARD;
	default:return fys::arena::FightingPit::EASY;
	}
}

fys::arena::AwaitingPlayerArena
createAwaitingPlayer(const fys::fb::FightingPitEncounter* binary)
{
	std::optional<fys::arena::AwaitingArena> awaitingArena = std::nullopt;

	// if fighting_pit_id is 0, a new arena has to be generated furthermore data are extracted
	if (!binary->fighting_pit_id()) {
		awaitingArena = fys::arena::AwaitingArena{
				binary->world_server_id()->str(),
				binary->is_ambush(),
				binary->id_encounter(),
				translateLevelFromFlatbuffer(binary->level_encounter())
		};
	}
	return fys::arena::AwaitingPlayerArena{
			binary->user_name()->str(),
			binary->token_auth()->str(),
			binary->fighting_pit_id(),
			std::move(awaitingArena)
	};
}

}

namespace fys::arena {

ArenaServerService::ArenaServerService(const ArenaServerContext& ctx)
		:
		_ctx(ctx),
		_cache(ctx.getPathLocalStorageCache(), ctx.getPathSourceCache()),
		_dbConnector(std::make_unique<network::DBConnector>(ctx))
{
	_connectionHandler.setupConnectionManager(ctx);
	_workerService.setupConnectionManager(ctx);
}

void
ArenaServerService::runServerLoop() noexcept
{

	SPDLOG_INFO("ArenaServer loop started");

	auto t = std::thread([this] { _workerService.startFightingPitsLoop(); });
	while (true) {
		_connectionHandler.pollAndProcessMessageFromDispatcher(
				[this](zmq::message_t&& identityWs, zmq::message_t&& worldServerMessage) {
					const auto* binary = fys::fb::GetFightingPitEncounter(worldServerMessage.data());

					AwaitingPlayerArena apa = createAwaitingPlayer(binary);

					if (!apa.hasToBeGenerated() && !_workerService.doesFightingPitExist(apa.fightingPitId)) {
						spdlog::error("Player {} can't be awaited to register on non-existing fighting pit {}",
								apa.namePlayer, apa.fightingPitId);
						return;
					}

					// register player incoming into arena instance with token given by worldserver used as key
					const auto &[elem, playerHasBeenCorrectlyRegistered] = _awaitingArena.insert({binary->token_auth()->str(), std::move(apa)});

					if (playerHasBeenCorrectlyRegistered) {
						forwardReplyToDispatcherClient(std::move(identityWs), elem->second);
						spdlog::info("A new awaited player is incoming {}", binary->token_auth()->str());
					}
				});

		_workerService.pollAndProcessMessageFromPlayer(
				// Authentication handler
				[this](zmq::message_t&& identityPlayer, zmq::message_t&& authMessage) {
					const auto* binary = fys::fb::GetArenaServerValidateAuth(authMessage.data());
					const std::string tokenAuth = binary->token_auth()->str();
					const std::string userName = binary->user_name()->str();
					const auto &[isAwaited, playerAwaitedIt] = isPlayerAwaited(userName, tokenAuth, binary->fighting_pit_id());
					unsigned fightingPitId = playerAwaitedIt->second.fightingPitId;

					if (!isAwaited) {
						spdlog::warn("Player {} tried to authenticate on Arena server {} fighting pit {} without being awaited.",
								userName, _ctx.get().getServerCode(), fightingPitId);
						return;
					}

					if (playerAwaitedIt->second.hasToBeGenerated()) {
						fightingPitId = createNewFightingPit(playerAwaitedIt->second);
						_awaitingArena.erase(playerAwaitedIt); // remove player from awaited player
						if (fightingPitId == FightingPit::CREATION_ERROR) return;
					}
					else {
						if (!_workerService.doesFightingPitExist(binary->fighting_pit_id())) {
							spdlog::error("Player {} tried to join fighting pit of id {} which doesn't exist", userName, binary->fighting_pit_id());
							return;
						}
						auto pt = _dbConnector->retrievePartyTeam(userName);
						_workerService.playerJoinFightingPit(fightingPitId, std::move(pt), _cache);
						_awaitingArena.erase(playerAwaitedIt); // remove player from awaited player
					}
					spdlog::info("Awaited player {} has logged in fighting pit of id:{}", userName, fightingPitId);
					_workerService.addPlayerIdentifier(fightingPitId, userName, identityPlayer.str());
					_workerService.broadCastNewArrivingTeam(fightingPitId, userName);
				},

				// InGame handler
				[this](zmq::message_t&& identityPlayer, const zmq::message_t& intermediate, zmq::message_t&& playerMsg) {
					zmq::multipart_t response;
					response.add(std::move(identityPlayer));
					const auto authFrame = fys::fb::GetArenaServerValidateAuth(intermediate.data());
					auto fp = _workerService.getAuthenticatedPlayerFightingPit(authFrame->user_name()->str(),
							authFrame->token_auth()->str(),
							authFrame->fighting_pit_id());

					if (!fp) {
						spdlog::warn("Player {}:{} is not authenticated.", authFrame->user_name()->str(), authFrame->token_auth()->str());
						return;
					}
					unsigned idMember = 0;
					// todo create an action message to forward
					spdlog::info("InGame Message received");
					fp->get().forwardMessageToTeamMember(authFrame->user_name()->str(), idMember);
				});
	}
	t.join();
}

std::pair<bool, const std::unordered_map<std::string, AwaitingPlayerArena>::const_iterator>
ArenaServerService::isPlayerAwaited(const std::string& name, const std::string& token, unsigned idFightingPit) const noexcept
{
	if (const auto it = _awaitingArena.find(token);
			it != _awaitingArena.end() && it->second.namePlayer == name && it->second.fightingPitId == idFightingPit)
		return {true, it};
	return {false, _awaitingArena.cend()};
}

void
ArenaServerService::forwardReplyToDispatcherClient(zmq::message_t&& identityWs, const fys::arena::AwaitingPlayerArena& awaitingArena) noexcept
{
	flatbuffers::FlatBufferBuilder fbb;
	auto asaFb = fys::fb::CreateArenaServerAuth(
			fbb,
			fbb.CreateString(awaitingArena.namePlayer),
			fbb.CreateString(awaitingArena.token),
			fbb.CreateString(_ctx.get().getHostname()),
			fbb.CreateString(_ctx.get().getConnectionString()),
			_ctx.get().getPort(),
			awaitingArena.gen ? fbb.CreateString(awaitingArena.gen->serverCode) : fbb.CreateString(""));
	fys::fb::FinishArenaServerAuthBuffer(fbb, asaFb);
	zmq::multipart_t msg;
	msg.add(std::move(identityWs));
	msg.addmem(fbb.GetBufferPointer(), fbb.GetSize());
	_connectionHandler.sendMessageToDispatcher(std::move(msg));
}

unsigned
ArenaServerService::createNewFightingPit(const AwaitingPlayerArena& awaited) noexcept
{
	FightingPitAnnouncer fpa(_cache);

	fpa.setDifficulty(awaited.gen->levelFightingPit);
	fpa.setEncounterId(awaited.gen->encounterId);
	fpa.enforceAmbush(awaited.gen->isAmbush);
	fpa.setCreatorUserName(awaited.namePlayer);
	fpa.setCreatorUserToken(awaited.token);
	fpa.setCreatorTeamParty(_dbConnector->retrievePartyTeam(awaited.namePlayer));

	SPDLOG_INFO("New fighting pit to be created lvl name {} token {} {} encounterid {} isAmbush {} wsCode {}",
			awaited.namePlayer, awaited.token,
			awaited.gen->levelFightingPit, awaited.gen->encounterId,
			awaited.gen->isAmbush, awaited.gen->serverCode);

	unsigned id = _workerService.addFightingPit(
			fpa.buildFightingPit(_ctx.get().getEncounterContext(), awaited.gen->serverCode));
	return id;
}

} // namespace fys::arena
