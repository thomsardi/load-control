#ifndef TALIS5_JSON_HANDLER_H
#define TALIS5_JSON_HANDLER_H

#include <stdint.h>
#include <ArduinoJson.h>
#include <vector>
#include <Talis5Memory.h>
#include <EthernetSave.h>

class Talis5JsonHandler
{
private:
    /* data */
    const char* _TAG = "Talis5 Json Handler";
public:
    Talis5JsonHandler(/* args */);
    String buildJsonResponse(int code);
    size_t parseSetSlaveJson(String body, std::vector<uint8_t>& buff);
    bool parseSetNetwork(String body, EthernetParameterData& ethernetParameterData);
    bool parseModbusScan(String body);
    bool parseSetModbus(String body, Talis5ParameterData& talis5ParameterData);
    bool parseSetSerialModbus(String body, Talis5ParameterData& talis5ParameterData);
    bool parseRestart(String body);
    bool parseFactoryReset(String body);
    ~Talis5JsonHandler();
};




#endif