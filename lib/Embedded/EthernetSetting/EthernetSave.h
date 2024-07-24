#ifndef ETHERNET_SAVE_H
#define ETHERNET_SAVE_H

#include <Arduino.h>
#include <Preferences.h>
#include "defs.h"
#include "LittleFS.h"

struct EthernetParameterData {
    uint8_t server; // 1 = STATIC, 2 = DHCP
    String ip;
    String gateway;
    String subnet;
    std::array<uint8_t, 6> mac;
};

class EthernetSave
{
private:
    /* data */
    const char* _TAG = "Ethernet Save";
    EthernetParameterData _shadowParameter;
    String _name;
    bool _isActive = false;
    void copy();
    void createDefault();
    void writeShadow();
    void resetFlag();
    bool _isServerSet = false;
    bool _isIpSet = false;
    bool _isGatewaySet = false;
    bool _isSubnetSet = false;
    bool _isMacSet = false;

public:
    EthernetSave();
    void printDefault();
    void printUser();
    
    void begin(String name);
    void save();
    void cancel();
    void reset();
    void restart();
    void clear();

    void setServer(uint8_t serverType);
    void setIp(String ip);
    void setGateway(String ip);
    void setSubnet(String ip);
    void setMac(uint8_t* mac, size_t arrLen);

    uint8_t getServer();
    String getIp();
    String getGateway();
    String getSubnet();
    size_t getMac(uint8_t* buff, size_t len);
    String getMacString();

    ~EthernetSave();
};


#endif