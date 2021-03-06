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

#include <catch2/catch.hpp>

#include <fightingPit/FightingPitAnnouncer.hh>
#include <network/WorkerService.hh>

TEST_CASE("WorkerService test", "[service][arena]")
{

	fys::arena::WorkerService ws;

	SECTION("Failures") {

		REQUIRE(fys::arena::FightingPit::CREATION_ERROR == ws.addFightingPit(nullptr));

//		for (int i = 0 ; i < std::numeric_limits<unsigned>::max(); ++i) {
//			unsigned id = ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("1", fys::arena::FightingPit::EASY));
//			REQUIRE((i + 1) == id);
//			REQUIRE("1" == fys::arena::FightingPitAnnouncer::getCreatorUserName(ws.getFightingPitInstance(id)));
//		}
//
//		REQUIRE(fys::arena::FightingPit::CREATION_ERROR == ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("1", fys::arena::FightingPit::EASY)));
	} // End section : Failures

	SECTION("Test addFightingPit") {

		unsigned id1 = ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("1", fys::arena::FightingPit::EASY));
		REQUIRE(1 == id1);
		REQUIRE("1" == fys::arena::FightingPitAnnouncer::getCreatorUserName(ws.getFightingPitInstance(id1)));

		unsigned id2 = ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("2", fys::arena::FightingPit::EASY));
		REQUIRE(2 == id2);
		REQUIRE("2" == fys::arena::FightingPitAnnouncer::getCreatorUserName(ws.getFightingPitInstance(id2)));

		unsigned id3 = ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("3", fys::arena::FightingPit::EASY));
		REQUIRE(3 == id3);
		REQUIRE("3" == fys::arena::FightingPitAnnouncer::getCreatorUserName(ws.getFightingPitInstance(id3)));

		unsigned id4 = ws.addFightingPit(std::make_unique<fys::arena::FightingPit>("4", fys::arena::FightingPit::EASY));
		REQUIRE(4 == id4);
		REQUIRE("4" == fys::arena::FightingPitAnnouncer::getCreatorUserName(ws.getFightingPitInstance(id4)));

	} // End section : Test addFightingPit

} // End TestCase : WorkerService test