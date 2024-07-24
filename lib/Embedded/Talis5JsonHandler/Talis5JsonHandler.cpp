#include "Talis5JsonHandler.h"

Talis5JsonHandler::Talis5JsonHandler(/* args */)
{
}

/**
 * Parse post json body for set slave api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * @param[in]   buff  vector of uint8_t to store the slave
 * 
 * @return  number of slave when success, 0 when failed or no input  
*/
size_t Talis5JsonHandler::parseSetSlaveJson(String body, std::vector<uint8_t>& buff)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    
    size_t len = 0;
    if (!json.containsKey("slave_list"))
    {
        return 0;
    }
    JsonArray arr = json["slave_list"];
    buff.reserve(arr.size());
    ESP_LOGI(_TAG, "arr size : %d\n", arr.size());
    for (size_t i = 0; i < arr.size(); i++)
    {
        JsonVariant value = arr[i];
        buff.push_back(value.as<uint8_t>());
    }
    for (size_t i = 0; i < buff.size(); i++)
    {
        ESP_LOGI(_TAG, "slave list : %d\n", buff.at(i));
    }
    len = arr.size();
    
    return len;
}

String Talis5JsonHandler::buildJsonResponse(int code)
{
    JsonDocument doc;
    String response;
    doc["status"] = code;
    serializeJson(doc, response);
    return response;
}

/**
 * Parse post json body for set network api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * @param[in]   wifiParameterData  WifiParameterData data structure
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseSetNetwork(String body, EthernetParameterData& ethernetParameterData)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    
    if (error) {
        ESP_LOGI(_TAG, "deserializeJson() failed: ");
        ESP_LOGI(_TAG, "%s\n", error.c_str());
        return 0;
    }
    
    if( !json.containsKey("server") || !json.containsKey("ip") ||
        !json.containsKey("gateway") || !json.containsKey("subnet") ||
        !json.containsKey("mac") )
    {
        return 0;
    }

    ethernetParameterData.server = json["server"];
    ethernetParameterData.ip = json["ip"].as<String>();
    ethernetParameterData.gateway = json["gateway"].as<String>();
    ethernetParameterData.subnet = json["subnet"].as<String>();
    JsonArray arr = json["mac"];
    if (arr.size() < 6)
    {
        return 0;
    }
    for (size_t i = 0; i < ethernetParameterData.mac.size(); i++)
    {
        ethernetParameterData.mac[i] = arr[i];
    }
    return 1;
}

/**
 * Parse post json body for set modbus scan api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseModbusScan(String body)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    
    if (error) {
        ESP_LOGI(_TAG, "deserializeJson() failed: ");
        ESP_LOGI(_TAG, "%s\n", error.c_str());
        return 0;
    }
    
    if (!json.containsKey("scan"))
    {
        return 0;
    }

    int scan = json["scan"];

    if (scan > 0)
    {
        return 1;
    }
    return 0;
}

/**
 * Parse post json body for set modbus network api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * @param[in]   talis5ParameterData Talis5ParameterData data structure
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseSetModbus(String body, Talis5ParameterData& talis5ParameterData)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    
    if (error) {
        ESP_LOGI(_TAG, "deserializeJson() failed: ");
        ESP_LOGI(_TAG, "%s\n", error.c_str());
        return 0;
    }

    if (!json.containsKey("ip") || !json.containsKey("port"))
    {
        return 0;
    }

    JsonVariant ip = json["ip"];
    JsonVariant port = json["port"];
    talis5ParameterData.modbusTargetIp = json["ip"].as<String>(); ip.as<String>();
    talis5ParameterData.modbusPort = port.as<int>();
    return 1;
}

/**
 * Parse post json body for set serial modbus api
 * 
 * @param[in]   body  jsonVariant with key:value pair json
 * @param[in]   talis5ParameterData Talis5ParameterData data structure
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseSetSerialModbus(String body, Talis5ParameterData& talis5ParameterData)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    
    if (error) {
        ESP_LOGI(_TAG, "deserializeJson() failed: ");
        ESP_LOGI(_TAG, "%s\n", error.c_str());
        return 0;
    }

    if (!json.containsKey("baudrate"))
    {
        return 0;
    }

    talis5ParameterData.baudRate = json["baudrate"];
    return 1;
}

/**
 * Parse post json body for restart api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseRestart(String body)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    if (!json.containsKey("restart"))
    {
        return 0;
    }

    int restart = json["restart"];

    if (restart > 0)
    {
        return 1;
    }
    return 0;
}

/**
 * Parse post json body for factory reset api
 * 
 * @param[in]   json  jsonVariant with key:value pair json
 * 
 * @return  true when success, false when failed  
*/
bool Talis5JsonHandler::parseFactoryReset(String body)
{
    JsonDocument json;
    DeserializationError error = deserializeJson(json, body);
    if (!json.containsKey("freset"))
    {
        return 0;
    }

    int freset = json["freset"];

    if (freset > 0)
    {
        return 1;
    }
    return 0;
}

Talis5JsonHandler::~Talis5JsonHandler()
{
}