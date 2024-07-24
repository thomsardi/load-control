#include "EthernetSave.h"

EthernetSave::EthernetSave()
{
}

/**
 * begin the preference namespace, write the default setting and init flag
 * 
 * @param[in]   name    name for preference namespace
*/
void EthernetSave::begin(String name)
{
    Preferences preferences;
    preferences.begin(name.c_str());
    _name = name;
    if (!preferences.isKey("init_flg")) // get init flag to detect the first time init
    {
        ESP_LOGI(_TAG, "Create default");
        createDefault();
        copy();
    }
    if (preferences.getBool("rst_flg")) // get reset flag to revert the user parameter into default
    {
        ESP_LOGI(_TAG, "Revert to default");
        copy();
        preferences.putBool("rst_flg", false);
    }
    _isActive = true;
    printDefault();
    printUser();
    writeShadow();
    resetFlag();
}

/**
 * create default parameter
*/
void EthernetSave::createDefault()
{
    // std::array<uint8_t, 6> mac = {0x02, 0x34, 0x56, 0x78, 0x9A, 0xBC};
    std::array<uint8_t, 6> mac = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUChar("d_svr", 2); // set to DHCP
    preferences.putString("d_ip", "192.168.4.1");
    preferences.putString("d_gateway", "192.168.4.1");
    preferences.putString("d_subnet", "255.255.255.0");
    preferences.putBytes("d_mac", mac.data(), mac.size());
    preferences.putBool("init_flg", true);
    preferences.putBool("rst_flg", false);
    preferences.end();
}

/**
 * copy default parameter into user parameter
*/
void EthernetSave::copy()
{
    std::array<uint8_t, 6> mac;
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUChar("u_svr", preferences.getUChar("d_svr"));
    preferences.putString("u_ip", preferences.getString("d_ip"));
    preferences.putString("u_gateway", preferences.getString("d_gateway"));
    preferences.putString("u_subnet", preferences.getString("d_subnet"));
    preferences.getBytes("d_mac", mac.data(), mac.size());
    preferences.putBytes("u_mac", mac.data(), mac.size());
    preferences.end();
}

/**
 * set ethernet server
 * 
 * @param[in]   serverType  1 = STATIC, 2 = DHCP
*/
void EthernetSave::setServer(uint8_t serverType)
{
    if (_isActive)
    {
        _shadowParameter.server = serverType;
        _isServerSet = true;
    }
}

/**
 * set wifi static IP
 * 
 * @param[in]   ip 
*/
void EthernetSave::setIp(String ip)
{
    if (_isActive)
    {
        _shadowParameter.ip = ip;
        _isIpSet = true;
    }
}

/**
 * set wifi gateway IP
 * 
 * @param[in]   ip 
*/
void EthernetSave::setGateway(String ip)
{
    if (_isActive)
    {
        _shadowParameter.gateway = ip;
        _isGatewaySet = true;
    }
}

/**
 * set wifi subnet mask
 * 
 * @param[in]   ip 
*/
void EthernetSave::setSubnet(String ip)
{
    if (_isActive)
    {
        _shadowParameter.subnet = ip;
        _isSubnetSet = true;
    }
}

/**
 * set mac
 * 
 * @param[in]   mac mac as array
 * @param[in]   arrLen  length of the array 
*/
void EthernetSave::setMac(uint8_t* mac, size_t arrLen)
{
    if (_isActive)
    {
        if (arrLen == 6)
        {
            memcpy(_shadowParameter.mac.data(), mac, arrLen);
            _isMacSet = true;
        }
    }
}

/**
 * write and save into preference memory
*/
void EthernetSave::save()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        if (_isServerSet)
        {
            preferences.putUChar("u_svr", _shadowParameter.server);
        }

        if (_isIpSet)
        {
            preferences.putString("u_ip", _shadowParameter.ip);
        }
        
        if (_isGatewaySet)
        {
            preferences.putString("u_gateway", _shadowParameter.gateway);
        }

        if (_isSubnetSet)
        {
            preferences.putString("u_subnet", _shadowParameter.subnet);
        }

        if(_isMacSet)
        {
            preferences.putBytes("u_mac", _shadowParameter.mac.data(), _shadowParameter.mac.size());
        }
        preferences.end();
        resetFlag();
    }
}

/**
 * write shadow register from user parameter
*/
void EthernetSave::writeShadow()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        _shadowParameter.server = preferences.getUChar("u_svr");
        _shadowParameter.ip = preferences.getString("u_ip");
        _shadowParameter.gateway = preferences.getString("u_gateway");
        _shadowParameter.subnet = preferences.getString("u_subnet");
        preferences.getBytes("u_mac", _shadowParameter.mac.data(), _shadowParameter.mac.size());
        preferences.end();
    }
}

/**
 * cancel the recent input and revert back into last known parameter
*/
void EthernetSave::cancel()
{
    writeShadow();
    resetFlag();
}

/**
 * reset into default parameter
*/
void EthernetSave::reset()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        preferences.putBool("rst_flg", true);
        preferences.end();
        _isActive = false;
        restart();
    }
}

/**
 * restart the object
*/
void EthernetSave::restart()
{
    begin(_name);
}

/**
 * clear all preferences keys
*/
void EthernetSave::clear()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        preferences.clear();
        preferences.end();
        _isActive = false;
        resetFlag();
        begin(_name);
    }
}


/**
 * Get server of the configuration
 * 
 * @return   server of the configuration, 1 for STATIC, 2 for DHCP
*/
uint8_t EthernetSave::getServer()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        uint8_t value = preferences.getUChar("u_svr");
        preferences.end();
        return value;
    }
    return 0;
}

/**
 * Get ip of the configuration
 * 
 * @return   ip of the configuration
*/
String EthernetSave::getIp()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        String value = preferences.getString("u_ip");
        preferences.end();
        return value;
    }
    return "";
}

/**
 * Get gateway ip of the configuration
 * 
 * @return   gateway ip of the configuration
*/
String EthernetSave::getGateway()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        String value = preferences.getString("u_gateway");
        preferences.end();
        return value;
    }
    return "";
}

/**
 * Get subnet mask of the configuration
 * 
 * @return   subnet mask of the configuration
*/
String EthernetSave::getSubnet()
{
    if (_isActive)
    {
        Preferences preferences;
        preferences.begin(_name.c_str());
        String value = preferences.getString("u_subnet");
        preferences.end();
        return value;
    }
    return "";
}

/**
 * Get mac string
 * 
 * @return   mac of the configuration
*/
String EthernetSave::getMacString()
{
    if (_isActive)
    {
        std::array<uint8_t, 6> mac;
        std::array<char, 20> buffer;
        Preferences preferences;
        preferences.begin(_name.c_str());
        preferences.getBytes("u_mac", mac.data(), mac.size());
        snprintf(buffer.data(), buffer.size(), "%02X::%02X::%02X::%02X::%02X::%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        String value = String(buffer.data());
        preferences.end();
        return value;
    }
    return "";
}

/**
 * Get mac
 * 
 * @return   mac of the configuration
*/
size_t EthernetSave::getMac(uint8_t* buff, size_t len)
{
    if (_isActive)
    {
        if (len < 6)
        {
            return 0;
        }
        Preferences preferences;
        preferences.begin(_name.c_str());
        preferences.getBytes("u_mac", buff, len);
        preferences.end();
        return len;
    }
    return 0;
}



/**
 * print default parameter
*/
void EthernetSave::printDefault()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    ESP_LOGI(_TAG, "d_svr : %d\n", preferences.getUChar("d_svr"));
    ESP_LOGI(_TAG, "d_ip : %s\n", preferences.getString("d_ip").c_str());
    ESP_LOGI(_TAG, "d_gateway : %s\n", preferences.getString("d_gateway").c_str());
    ESP_LOGI(_TAG, "d_subnet : %s\n", preferences.getString("d_subnet").c_str());
    std::array<uint8_t, 6> mac;
    preferences.getBytes("d_mac", mac.data(), mac.size());
    std::array<char, 32> buff;
    snprintf(buff.data(), buff.size(), "%02X::%02X::%02X::%02X::%02X::%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(_TAG, "d_mac : %s\n", buff.data());
    preferences.end();
}

/**
 * print user parameter
*/
void EthernetSave::printUser()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    ESP_LOGI(_TAG, "u_svr : %d\n", preferences.getUChar("u_svr"));
    ESP_LOGI(_TAG, "u_ip : %s\n", preferences.getString("u_ip").c_str());
    ESP_LOGI(_TAG, "u_gateway : %s\n", preferences.getString("u_gateway").c_str());
    ESP_LOGI(_TAG, "u_subnet : %s\n", preferences.getString("u_subnet").c_str());
    std::array<uint8_t, 6> mac;
    preferences.getBytes("u_mac", mac.data(), mac.size());
    std::array<char, 32> buff;
    snprintf(buff.data(), buff.size(), "%02X::%02X::%02X::%02X::%02X::%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    ESP_LOGI(_TAG, "u_mac : %s\n", buff.data());
    preferences.end();
}

void EthernetSave::resetFlag()
{
    _isServerSet = false;
    _isIpSet = false;
    _isGatewaySet = false;
    _isSubnetSet = false;
    _isMacSet = false;
}

EthernetSave::~EthernetSave()
{
}