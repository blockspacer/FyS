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

#include <nlohmann/json.hpp>

#include <algorithm/algorithm.hh>
#include <Cml.hh>
#include <ArenaServerContext.hh>
#include <ConnectionHandler.hh>
#include <RandomGenerator.hh>
#include <fightingPit/contender/ContenderScripting.hh>
#include <fightingPit/contender/FightingContender.hh>
#include <fightingPit/FightingPitAnnouncer.hh>
#include <fightingPit/FightingPit.hh>
#include <BoundaryMap.hh>

namespace {
    using BoundaryMapEncounter = fys::BoundaryMap<fys::arena::EncounterContext::EncounterDesc>;

    BoundaryMapEncounter
    makeContenderRngBoundaryMap(const std::vector<fys::arena::EncounterContext::EncounterDesc> &zoneContenders,
                                fys::arena::FightingPit::Level difficulty) {
        BoundaryMapEncounter bm;
        int val = 0;
        for (const auto &zc : zoneContenders) {
            val += zc.chance.at(static_cast<int>(difficulty));
            bm.insert(val, zc);
        }
        return bm;
    }

    std::string getNameFromKey(const std::string &key) {
        return key.substr(key.find_last_of(':') + 1, key.find_last_of('.') - key.find_last_of(':') - 1);
    }
}

namespace fys::arena {
    using json = nlohmann::json;

    std::shared_ptr<FightingPit>
    FightingPitAnnouncer::buildFightingPit(const EncounterContext &ctx, ConnectionHandler &connectionHandler,
                                           const std::string &wsId) {
        std::shared_ptr<FightingPit> fp = std::make_shared<FightingPit>(_difficulty);
        generateContenders(*fp, ctx, wsId);
        return fp;
    }

    void FightingPitAnnouncer::generateContenders(FightingPit &fp, const EncounterContext &ctx,
                                                  const std::string &wsId) {
        const auto &range = ctx._rangeEncounterPerZone.at(wsId).at(static_cast<std::size_t>(_difficulty));
        unsigned numberContenders = fys::util::RandomGenerator::generateInRange(range.first, range.second);
        auto boundaryMap = makeContenderRngBoundaryMap(ctx._contendersPerZone.at(wsId), _difficulty);

        for (int i = 0; i < numberContenders; ++i) {
            int rngMonster = fys::util::RandomGenerator::generateInRange(0, 100);
            auto desc = boundaryMap.get(rngMonster)->second; // TODO add security at context creation to enforce that
            int levelMonster = fys::util::RandomGenerator::generateInRange(1, 10); // TODO, add this range in the configuration
            auto contenderScript = std::make_unique<ContenderScripting>(*fp.getChaiPtr(), levelMonster);
            contenderScript->setContenderId(i);
            contenderScript->setContenderName(getNameFromKey(desc.key));
            contenderScript->loadContenderScript(_cache.findInCache(desc.key).data());
            fp.addContender(std::make_shared<FightingContender>(std::move(contenderScript)));
        }
    }

    void FightingPitAnnouncer::generatePartyTeams(const std::string &userName) {
        // TODO get data from DB to initialize party team
    }

} // namespace fys::arena
