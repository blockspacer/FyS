#include <spdlog/spdlog.h>
#include <tclap/ValueArg.h>
#include <tclap/CmdLine.h>
#include <boost/property_tree/ini_parser.hpp>
#include <StartupDispatcherCtx.hh>
#include <Versioner.hh>

fys::StartupDispatcherCtx::StartupDispatcherCtx(int ac, const char *const *av) try {
    _version = std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR);
    TCLAP::CmdLine cli("FyS::Dispatcher", ' ', _version);
    TCLAP::ValueArg<std::string> configPath("c", "config", "Path of config file", false, "NONE", "string");
    TCLAP::ValueArg<std::string> name("n", "name", "Name of the Dispatcher (used as key for the cluster)", false, "WS", "string");
    TCLAP::ValueArg<ushort> changePort("p", "port", "Listening Port", false, 0, "integer");
    TCLAP::ValueArg<bool> aware("a", "aware", "Is aware of the other cluster member", false, true, "boolean");
    TCLAP::ValueArg<bool> loadBalance("l", "load_balance", "Dispatch in load balancing mode or publisher mode", false, true, "boolean");
    TCLAP::ValueArg<bool> verbose("v", "verbose", "Print logs on standard output", false, true, "boolean");

    cli.add(configPath);
    cli.add(changePort);
    cli.add(name);
    cli.add(aware);
    cli.add(verbose);
    cli.parse(ac, av);
    if ("NONE" != configPath.getValue())
        this->initializeFromIni(configPath.getValue());
    if (_bindingPort > 0)
        _bindingPort = changePort.getValue();
    if (_name.empty())
        _name = name.getValue();
    _isClusterAware = !aware.getValue() ? false : _isClusterAware;
    _isLoadBalancing = loadBalance.getValue();
    _verbose = verbose.getValue();
}
catch (std::exception &e) {
    spdlog::get("c")->error("\"Context of the Dispatcher not initialized caused by : {}", e.what());
}


void fys::StartupDispatcherCtx::initializeFromIni(const std::string &configFilePath) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_ini(configFilePath, pt);

    _clusterProxy.backendAddress = pt.get<std::string>(fys::init_beacon::BACKEND_ADDR);
    _clusterProxy.backendPort= pt.get<ushort>(fys::init_beacon::BACKEND_PORT);
    _clusterProxy.frontendAddress = pt.get<std::string>(fys::init_beacon::FRONTEND_ADDR);
    _clusterProxy.frontendPort = pt.get<ushort>(fys::init_beacon::FRONTEND_PORT);
    _bindingPort = pt.get<ushort>(fys::init_beacon::BINDINGPORT);
    _isClusterAware = pt.get<bool>(fys::init_beacon::ISCLUSTERAWARE);
    _isLoadBalancing = pt.get<bool>(fys::init_beacon::ISLOADBALANCING);
    _name = pt.get<std::string>(fys::init_beacon::NAME);
    _subTopics = asVector<std::string>(pt, fys::init_beacon::TOPICS);
}

std::string fys::StartupDispatcherCtx::toString() const {
    std::string str;
    str = "\n*************************\n";
    str+= "[INFO] Dispatcher " + _name + " context VERSION : " + _version + "\n\n";
    str+= "[INFO] Listener bindingPort " + std::to_string(_bindingPort) + "\n";
    str+= "[INFO] isClusterAware : " + std::string(_isClusterAware ? "true" : "false") + "\n";
    str+= "[INFO] isLoadBalancing : " + std::string(_isLoadBalancing ? "true" : "false") + "\n";
    for (const auto &topic : _subTopics) {
        str+= "[INFO] Cluster: Subscribing topic :" + topic + "\n";
    }
    str+= "[INFO] Frontend connection string : " + getFrontendClusterProxyConnectionString() + "\n";
    str+= "[INFO] Backend connection string : " + getBackendClusterProxyConnectionString() + "\n";
    str+= "\n*************************\n";
    return str;
}

std::string fys::StartupDispatcherCtx::getFrontendClusterProxyConnectionString() const {
    return "tcp://" + _clusterProxy.frontendAddress + ":" + std::to_string(_clusterProxy.frontendPort);
}

std::string fys::StartupDispatcherCtx::getBackendClusterProxyConnectionString() const {
    return "tcp://" + _clusterProxy.backendAddress + ":" + std::to_string(_clusterProxy.backendPort);
}
