#include <zmq.h>
#include <DispatcherConnectionManager.hh>
#include <StartupDispatcherCtx.hh>

fys::network::DispatcherConnectionManager::DispatcherConnectionManager(int threadNumber, const std::string &proxyFeConnectionString, const std::string &proxyBeConnectionString) :
 _zmqContext(threadNumber), _clusterConnection({zmq::socket_t(_zmqContext, ZMQ_SUB), zmq::socket_t(_zmqContext, ZMQ_PUB)}) {
     _clusterConnection.pubSocket.connect(proxyFeConnectionString);
     _clusterConnection.subSocket.connect(proxyBeConnectionString);
}

void fys::network::DispatcherConnectionManager::setupConnectionManager(const fys::StartupDispatcherCtx &ctx) {
    subscribeToTopics(ctx.getSubscriptionTopics());
    _items[0] = { _clusterConnection.subSocket, 0, ZMQ_POLLIN, 0 };
}


void fys::network::DispatcherConnectionManager::subscribeToTopics(const std::vector<std::string> &topics) {
    for (const auto &topic : topics) {
        _clusterConnection.subSocket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.size());
    }
}
