/** © Copyright 2022 CERN
 *
 * This software is distributed under the terms of the
 * GNU Lesser General Public Licence version 3 (LGPL Version 3),
 * copied verbatim in the file “LICENSE”
 *
 * In applying this licence, CERN does not waive the privileges
 * and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 *
 * Author: Adrien Ledeul (HSE)
 *
 **/


#ifndef S7200HWSERVICE_H_
#define S7200HWSERVICE_H_

#include <HWService.hxx>
#include <memory>
#include "S7200LibFacade.hxx"

#include "Common/Logger.hxx"
#include "Common/AsyncRecurringTask.hxx"
#include <queue>
#include <chrono>
#include <unordered_map>

class S7200HWService : public HWService
{
  public:
    S7200HWService();
    virtual PVSSboolean initialize(int argc, char *argv[]);
    virtual PVSSboolean start();
    virtual void stop();
    virtual void workProc();
    virtual PVSSboolean writeData(HWObject *objPtr);



private:
    void handleConsumerConfigError(const std::string&, int, const std::string&);

    void handleConsumeNewMessage(const std::string&, const std::string&, char*);
    void handleNewIPAddress(const std::string& ip);

    errorCallbackConsumer _configErrorConsumerCB{[this](const std::string& ip, int err, const std::string& reason) { this->handleConsumerConfigError(ip, err, reason);}};
    consumeCallbackConsumer  _configConsumeCB{[this](const std::string& ip, const std::string& var, char* payload){this->handleConsumeNewMessage(ip, var, std::move(payload));}};
    std::function<void(const std::string&)> _newIPAddressCB{[this](const std::string& ip){this->handleNewIPAddress(ip);}};


    //Common
    void insertInDataToDp(CharString&& address, char* value);
    std::mutex _toDPmutex;

    std::queue<std::pair<CharString,char*>> _toDPqueue;
    std::atomic<bool> _brokersDown{false};

    enum
    {
       ADDRESS_OPTIONS_IP = 0,
       ADDRESS_OPTIONS_VAR,
       ADDRESS_OPTIONS_SIZE
    } ADDRESS_OPTIONS;

    std::unique_ptr<Common::AsyncRecurringTask<std::function<void()>>> _streamAsyncTask;
    std::unique_ptr<Common::AsyncRecurringTask<std::function<void()>>> _poolAsyncTask;

    std::vector<std::thread> _pollingThreads;

    std::map<std::string, S7200LibFacade*> _facades;
};


void handleSegfault(int signal_code);

#endif
