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
#include <iostream>
#include <fstream>
#include "WorldServerContext.hh"

using json = nlohmann::json;

namespace fys::ws {

    WorldServerContext::WorldServerContext(int ac, const char *const *av) :
        common::ServiceContextBase(ac, av) {
        std::cout << "OPOPOPOP : " << _configFile << "\n";
        std::ifstream i(_configFile);
        json jsonConfig;
        i >> jsonConfig;
        initWsContextWithJson(jsonConfig);
    }

    void WorldServerContext::initWsContextWithJson(json &json) {
        auto wsJson = json["worldServer"];
        auto confJson = wsJson["conf"];
        auto overlapsJson = confJson["overlapServer"];

        wsJson["code"].get_to(_serverCode);
        _serverXBoundaries = std::make_pair(confJson["begin_x"].get<double>(), confJson["end_x"].get<double>());
        _serverYBoundaries = std::make_pair(confJson["begin_y"].get<double>(), confJson["end_y"].get<double>());
        for (auto &[key, value] : overlapsJson.items()) {
            ProximityServerAxis proximityServer;
            proximityServer.code = value["code"].get<std::string>();

            if (auto proxiXJson = value["overlap_x"]; !proxiXJson.is_null()) {
                proximityServer.superiorTo = proxiXJson["condition"].get<std::string>().find(">") != std::string::npos;
                proxiXJson["value"].get_to(proximityServer.value);
                _xAxisServerProximity.push_back(proximityServer);
            }
            if (auto proxiYJson = value["overlap_y"]; !proxiYJson.is_null()) {
                proximityServer.superiorTo = proxiYJson["condition"].get<std::string>().find(">") != std::string::npos;
                proxiYJson["value"].get_to(proximityServer.value);
                _yAxisServerProximity.push_back(std::move(proximityServer));
            }
        }
    }

    std::string WorldServerContext::toString() const noexcept {
        std::string str;
        str = "\n*************************\n";
        str+= "[INFO] Service " + _name + " context VERSION: " + _version + "\n";
        str+= "[INFO] Config file used: " + _configFile + "\n\n";
        str+= "[INFO] World Server code: " + _serverCode + "\n";
        str+= "[INFO] Dispatcher subscribing port: " + std::to_string(_dispatcherData.subscriberPort) + "\n";
        str+= "[INFO] Dispatcher connected port: " + std::to_string(_dispatcherData.port) + "\n";
        str+= "[INFO] Dispatcher connected host: " + _dispatcherData.address + "\n";
        str+= "\n====================\n[INFO] XAxisProximity elements\n";
        for (const auto &prox : _xAxisServerProximity) {
            str+= "[INFO] element:\n";
            str+= "code: " + prox.code + "\n";
            str+= "value: " + std::to_string(prox.value) + "\n";
            str+= "isSup: " + std::to_string(prox.superiorTo) + "\n";
        }
        str+= "\n====================\n[INFO] YAxisProximity elements\n";
        for (const auto &prox : _yAxisServerProximity) {
            str+= "[INFO] element:\n";
            str+= "code: " + prox.code + "\n";
            str+= "value: " + std::to_string(prox.value) + "\n";
            str+= "isSup: " + std::to_string(prox.superiorTo) + "\n";
        }
        str+= "\n*************************\n";
        return str;
    }

    std::string WorldServerContext::getDispatcherConnectionString() const noexcept {
        return std::string("tcp://").append(_dispatcherData.address).append(":").append(std::to_string(_dispatcherData.port));
    }

    std::string WorldServerContext::getDispatcherSubConnectionString() const noexcept {
        return std::string("tcp://").append(_dispatcherData.address).append(":").append(std::to_string(_dispatcherData.subscriberPort));
    }

}