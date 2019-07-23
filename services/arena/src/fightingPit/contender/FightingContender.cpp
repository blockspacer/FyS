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
#include <fightingPit/contender/FightContender.hh>

namespace {
    static constexpr RightLeft RIGHT = true;
    static constexpr RightLeft LEFT  = false;
}

namespace fys::arena {

    FightingContender::FightingContender(std::unique_ptr<ContenderScripting> && contenderScripting)  : _contenderScripting(std::move(contenderScripting)) {
        _contenderScripting->registerFunction(this, &fys::arena::FightingContender::moveContender, "moveContender");
    }

    void FightingContender::moveContender(HexagonSide::Orientation destination, bool bypassCheck) {
        if (!_side.move(destination, bypassCheck)) {
            SPDLOG_ERROR("Impossible move from {} to {}", _side, destination);
            return;
        }
    }

    void FightingContender::moveContender(data::MoveDirection directionToMove) {
        if (rightOrLeft == RIGHT) {
            if (!_side.move(destination)) {
                SPDLOG_ERROR("Impossible move from {} to right", _side);
            }
        }
        else (rightOrLeft == LEFT) {
            if (!_side.move(destination)) {
                SPDLOG_ERROR("Impossible move from {} to left", _side);
            }
        }
    }

}
