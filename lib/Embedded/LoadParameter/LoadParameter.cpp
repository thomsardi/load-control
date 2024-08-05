#include "LoadParameter.h"

LoadParameter::LoadParameter(/* args */)
{
}

/**
 * begin the preference namespace, write the default setting and init flag
 * 
 * @param[in]   name    name for preference namespace
*/
void LoadParameter::begin(String name)
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
    printDefault();
    printUser();
    writeShadow();
}

/**
 * create default parameter
*/
void LoadParameter::createDefault()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("d_baud", 6);  // default baudrate
    preferences.putUShort("d_id", 254); // default id
    preferences.putUShort("d_ov_d1", 600);  // default overvoltage disconnect
    preferences.putUShort("d_ov_r1", 580);  // default overcoltage reconnect
    preferences.putUShort("d_uv_d1", 500);  // default undervoltage disconnect
    preferences.putUShort("d_uv_r1", 510);  // default undervoltage reconnect
    preferences.putUShort("d_oc_d1", 1000); // default overcurrent disconnect
    preferences.putUShort("d_oc_dt1", 500);    // default overcurrent detection time
    preferences.putUShort("d_oc_rt1", 4000);    // default overcurrent reconnect time
    preferences.putUShort("d_om_1", 0);    // default output mode
    preferences.putUShort("d_ov_d2", 600);  // default overvoltage disconnect
    preferences.putUShort("d_ov_r2", 580);  // default overcoltage reconnect
    preferences.putUShort("d_uv_d2", 500);  // default undervoltage disconnect
    preferences.putUShort("d_uv_r2", 510);  // default undervoltage reconnect
    preferences.putUShort("d_oc_d2", 1000); // default overcurrent disconnect
    preferences.putUShort("d_oc_dt2", 500);    // default overcurrent detection time
    preferences.putUShort("d_oc_rt2", 4000);    // default overcurrent reconnect time
    preferences.putUShort("d_om_2", 0);    // default output mode
    preferences.putUShort("d_ov_d3", 600);  // default overvoltage disconnect
    preferences.putUShort("d_ov_r3", 580);  // default overcoltage reconnect
    preferences.putUShort("d_uv_d3", 500);  // default undervoltage disconnect
    preferences.putUShort("d_uv_r3", 510);  // default undervoltage reconnect
    preferences.putUShort("d_oc_d3", 1000); // default overcurrent disconnect
    preferences.putUShort("d_oc_dt3", 500);    // default overcurrent detection time
    preferences.putUShort("d_oc_rt3", 4000);    // default overcurrent reconnect time
    preferences.putUShort("d_om_3", 0);    // default output mode
    preferences.putBool("init_flg", true);
    preferences.putBool("rst_flg", false);
    preferences.end();
}

/**
 * copy default parameter into user parameter
*/
void LoadParameter::copy()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_baud", preferences.getUShort("d_baud"));
    preferences.putUShort("u_id", preferences.getUShort("d_id"));
    preferences.putUShort("u_ov_d1", preferences.getUShort("d_ov_d1"));  
    preferences.putUShort("u_ov_r1", preferences.getUShort("d_ov_r1"));  
    preferences.putUShort("u_uv_d1", preferences.getUShort("d_uv_d1"));  
    preferences.putUShort("u_uv_r1", preferences.getUShort("d_uv_r1"));  
    preferences.putUShort("u_oc_d1", preferences.getUShort("d_oc_d1")); 
    preferences.putUShort("u_oc_dt1", preferences.getUShort("d_oc_dt1"));    
    preferences.putUShort("u_oc_rt1", preferences.getUShort("d_oc_rt1"));    
    preferences.putUShort("u_om_1", preferences.getUShort("d_om_1"));    

    preferences.putUShort("u_ov_d2", preferences.getUShort("d_ov_d2"));  
    preferences.putUShort("u_ov_r2", preferences.getUShort("d_ov_r2"));  
    preferences.putUShort("u_uv_d2", preferences.getUShort("d_uv_d2"));  
    preferences.putUShort("u_uv_r2", preferences.getUShort("d_uv_r2"));  
    preferences.putUShort("u_oc_d2", preferences.getUShort("d_oc_d2")); 
    preferences.putUShort("u_oc_dt2", preferences.getUShort("d_oc_dt2"));    
    preferences.putUShort("u_oc_rt2", preferences.getUShort("d_oc_rt2"));    
    preferences.putUShort("u_om_2", preferences.getUShort("d_om_2"));

    preferences.putUShort("u_ov_d3", preferences.getUShort("d_ov_d3"));  
    preferences.putUShort("u_ov_r3", preferences.getUShort("d_ov_r3"));  
    preferences.putUShort("u_uv_d3", preferences.getUShort("d_uv_d3"));  
    preferences.putUShort("u_uv_r3", preferences.getUShort("d_uv_r3"));  
    preferences.putUShort("u_oc_d3", preferences.getUShort("d_oc_d3")); 
    preferences.putUShort("u_oc_dt3", preferences.getUShort("d_oc_dt3"));    
    preferences.putUShort("u_oc_rt3", preferences.getUShort("d_oc_rt3"));   
    preferences.putUShort("u_om_3", preferences.getUShort("d_om_3"));
    preferences.end();
}

/**
 * write into shadow register
*/
void LoadParameter::writeShadow()
{
    Preferences preferences;
    preferences.begin(_name.c_str());

    _shadowRegisters[0] = preferences.getUShort("u_baud");
    _shadowRegisters[1] = preferences.getUShort("u_id");

    _shadowRegisters[2] = preferences.getUShort("u_ov_d1");
    _shadowRegisters[3] = preferences.getUShort("u_ov_r1");
    _shadowRegisters[4] = preferences.getUShort("u_uv_d1");
    _shadowRegisters[5] = preferences.getUShort("u_uv_r1");
    _shadowRegisters[6] = preferences.getUShort("u_oc_d1");
    _shadowRegisters[7] = preferences.getUShort("u_oc_dt1");
    _shadowRegisters[8] = preferences.getUShort("u_oc_rt1");
    _shadowRegisters[9] = preferences.getUShort("u_om_1");

    _shadowRegisters[10] = preferences.getUShort("u_ov_d2");
    _shadowRegisters[11] = preferences.getUShort("u_ov_r2");
    _shadowRegisters[12] = preferences.getUShort("u_uv_d2");
    _shadowRegisters[13] = preferences.getUShort("u_uv_r2");
    _shadowRegisters[14] = preferences.getUShort("u_oc_d2");
    _shadowRegisters[15] = preferences.getUShort("u_oc_dt2");
    _shadowRegisters[16] = preferences.getUShort("u_oc_rt2");
    _shadowRegisters[17] = preferences.getUShort("u_om_2");

    _shadowRegisters[18] = preferences.getUShort("u_ov_d3");
    _shadowRegisters[19] = preferences.getUShort("u_ov_r3");
    _shadowRegisters[20] = preferences.getUShort("u_uv_d3");
    _shadowRegisters[21] = preferences.getUShort("u_uv_r3");
    _shadowRegisters[22] = preferences.getUShort("u_oc_d3");
    _shadowRegisters[23] = preferences.getUShort("u_oc_dt3");
    _shadowRegisters[24] = preferences.getUShort("u_oc_rt3");
    _shadowRegisters[25] = preferences.getUShort("u_om_3");

    preferences.end();
}

/**
 * write single value into shadow register and update flash
 * 
 * @param[in]   index   start index
 * @param[in]   value   value to be written
*/
void LoadParameter::writeSingle(size_t index, uint16_t value)
{
    if (index > _shadowRegisters.size() - 1)
    {
        return;
    }

    if (_shadowRegisters[index] != value)
    {
        switch (index)
        {
        case 0:
            setBaudrate(value);
            break;
        case 1:
            setId(value);
            break;
        case 2:
            setOvervoltageDisconnect1(value);
            break;
        case 3:
            setOvervoltageReconnect1(value);
            break;
        case 4:
            setUndervoltageDisconnect1(value);
            break;
        case 5:
            setUndervoltageReconnect1(value);
            break;
        case 6:
            setOvercurrentDisconnect1(value);
            break;
        case 7:
            setOvercurrentDetectionTime1(value);
            break;
        case 8:
            setOvercurrentReconnectInterval1(value);
            break;
        case 9 :
            setOutputMode1(value);
            break;
        case 10:
            setOvervoltageDisconnect2(value);
            break;
        case 11:
            setOvervoltageReconnect2(value);
            break;
        case 12:
            setUndervoltageDisconnect2(value);
            break;
        case 13:
            setUndervoltageReconnect2(value);
            break;
        case 14:
            setOvercurrentDisconnect2(value);
            break;
        case 15:
            setOvercurrentDetectionTime2(value);
            break;
        case 16:
            setOvercurrentReconnectInterval2(value);
            break;
        case 17:
            setOutputMode2(value);
            break;
        case 18:
            setOvervoltageDisconnect3(value);
            break;
        case 19:
            setOvervoltageReconnect3(value);
            break;
        case 20:
            setUndervoltageDisconnect3(value);
            break;
        case 21:
            setUndervoltageReconnect3(value);
            break;
        case 22:
            setOvercurrentDisconnect3(value);
            break;
        case 23:
            setOvercurrentDetectionTime3(value);
            break;
        case 24:
            setOvercurrentReconnectInterval3(value);
            break;
        case 25:
            setOutputMode3(value);
            break;
        default:
            break;
        }
    }
    writeShadow();
}

/**
 * write multiple value into shadow register
 * 
 * @param[in]   startIndex   start index
 * @param[in]   buffSize   size of array
 * @param[in]   buff    array of values
 * 
 * @return  number of register written
*/
size_t LoadParameter::writeMultiple(size_t startIndex, size_t buffSize, uint16_t *buff)
{
    if (startIndex + buffSize > _shadowRegisters.size())
    {
        return 0;
    }
    uint16_t numberWritten = 0;
    for (size_t i = 0; i < buffSize; i++)
    {
        size_t shadowIndex = i + startIndex;
        writeSingle(shadowIndex, buff[i]);
        numberWritten++;
    }
    writeShadow();
    return numberWritten;
}

/**
 * reset into default parameter
*/
void LoadParameter::reset()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putBool("rst_flg", true);
    preferences.end();
    restart();
}

/**
 * restart the object
*/
void LoadParameter::restart()
{
    begin(_name);
}

/**
 * clear all preferences keys
*/
void LoadParameter::clear()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.clear();
    preferences.end();
    begin(_name);    
}


/**
 * get baud rate
 * 
 * @return  baud rate of configuration
*/
uint16_t LoadParameter::getBaudrate()
{
    return _shadowRegisters[0];
}

/**
 * get baud rate in bps value (9600 - 115200)
 * 
 * @return  baud rate in bps
*/
int LoadParameter::getBaudrateBps()
{
    uint16_t val = _shadowRegisters[0];
    switch (val)
    {
    case 0:
        return 9600;
        break;
    case 1:
        return 14400;
        break;
    case 2:
        return 19200;
        break;
    case 3:
        return 28800;
        break;
    case 4:
        return 38400;
        break;
    case 5:
        return 57600;
        break;
    case 6:
        return 115200;
        break;    
    default:
        break;
    }
    return 0;
}

/**
 * get id
 * 
 * @return  id
*/
uint16_t LoadParameter::getId()
{
    return _shadowRegisters[1];
}

/**
 * get load 1 overvoltage disconnect
 * 
 * @return  overvoltage disconnect value in 0.1V (600 = 60.0V)
*/
uint16_t LoadParameter::getOvervoltageDisconnect1()
{
    return _shadowRegisters[2];
}

/**
 * get load 1 overvoltage reconnect
 * 
 * @return  overvoltage reconnect value in 0.1V (580 = 58.0V)
*/
uint16_t LoadParameter::getOvervoltageReconnect1()
{
    return _shadowRegisters[3];
}

/**
 * get load 1 undervoltage disconnect
 * 
 * @return  undervoltage disconnect value in 0.1V (500 = 50.0V)
*/
uint16_t LoadParameter::getUndervoltageDisconnect1()
{
    return _shadowRegisters[4];
}

/**
 * get load 1 undervoltage reconnect
 * 
 * @return  undervoltage reconnect value in 0.1V (510 = 51.0V)
*/
uint16_t LoadParameter::getUndervoltageReconnect1()
{
    return _shadowRegisters[5];
}

/**
 * get load 1 overcurrent disconnect
 * 
 * @return  overcurrent disconnect value in 0.01A (1250 = 12.5A)
*/
uint16_t LoadParameter::getOvercurrentDisconnect1()
{
    return _shadowRegisters[6];
}

/**
 * get load 1 overcurrent detection time
 * 
 * @return  overcurrent detection time value in ms
*/
uint16_t LoadParameter::getOvercurrentDetectionTime1()
{
    return _shadowRegisters[7];
}

/**
 * get load 1 overcurrent reconnect interval time
 * 
 * @return  overcurrent reconnect interval time value in ms
*/
uint16_t LoadParameter::getOvercurrentReconnectInterval1()
{
    return _shadowRegisters[8];
}

/**
 * get output mode 1
 * 
 * @return  output mode, 1 = source, 0 = sink
*/
uint16_t LoadParameter::getOutputMode1()
{
    return _shadowRegisters[9];
}

/**
 * get load 2 overvoltage disconnect
 * 
 * @return  overvoltage disconnect value in 0.1V (600 = 60.0V)
*/
uint16_t LoadParameter::getOvervoltageDisconnect2()
{
    return _shadowRegisters[10];
}

/**
 * get load 2 overvoltage reconnect
 * 
 * @return  overvoltage reconnect value in 0.1V (580 = 58.0V)
*/
uint16_t LoadParameter::getOvervoltageReconnect2()
{
    return _shadowRegisters[11];
}

/**
 * get load 2 undervoltage disconnect
 * 
 * @return  undervoltage disconnect value in 0.1V (500 = 50.0V)
*/
uint16_t LoadParameter::getUndervoltageDisconnect2()
{
    return _shadowRegisters[12];
}

/**
 * get load 2 undervoltage reconnect
 * 
 * @return  undervoltage reconnect value in 0.1V (510 = 51.0V)
*/
uint16_t LoadParameter::getUndervoltageReconnect2()
{
    return _shadowRegisters[13];
}

/**
 * get load 2 overcurrent disconnect
 * 
 * @return  overcurrent disconnect value in 0.01A (1250 = 12.5A)
*/
uint16_t LoadParameter::getOvercurrentDisconnect2()
{
    return _shadowRegisters[14];
}

/**
 * get load 2 overcurrent detection time
 * 
 * @return  overcurrent detection time value in ms
*/
uint16_t LoadParameter::getOvercurrentDetectionTime2()
{
    return _shadowRegisters[15];
}

/**
 * get load 2 overcurrent reconnect interval time
 * 
 * @return  overcurrent reconnect interval time value in ms
*/
uint16_t LoadParameter::getOvercurrentReconnectInterval2()
{
    return _shadowRegisters[16];
}

/**
 * get output mode 2
 * 
 * @return  output mode, 1 = source, 0 = sink
*/
uint16_t LoadParameter::getOutputMode2()
{
    return _shadowRegisters[17];
}

/**
 * get load 3 overvoltage disconnect
 * 
 * @return  overvoltage disconnect value in 0.1V (600 = 60.0V)
*/
uint16_t LoadParameter::getOvervoltageDisconnect3()
{
    return _shadowRegisters[18];
}

/**
 * get load 3 overvoltage reconnect
 * 
 * @return  overvoltage reconnect value in 0.1V (580 = 58.0V)
*/
uint16_t LoadParameter::getOvervoltageReconnect3()
{
    return _shadowRegisters[19];
}

/**
 * get load 3 undervoltage disconnect
 * 
 * @return  undervoltage disconnect value in 0.1V (500 = 50.0V)
*/
uint16_t LoadParameter::getUndervoltageDisconnect3()
{
    return _shadowRegisters[20];
}

/**
 * get load 3 undervoltage reconnect
 * 
 * @return  undervoltage reconnect value in 0.1V (510 = 51.0V)
*/
uint16_t LoadParameter::getUndervoltageReconnect3()
{
    return _shadowRegisters[21];
}

/**
 * get load 3 overcurrent disconnect
 * 
 * @return  overcurrent disconnect value in 0.01A (1250 = 12.5A)
*/
uint16_t LoadParameter::getOvercurrentDisconnect3()
{
    return _shadowRegisters[22];
}

/**
 * get load 3 overcurrent detection time
 * 
 * @return  overcurrent detection time value in ms
*/
uint16_t LoadParameter::getOvercurrentDetectionTime3()
{
    return _shadowRegisters[23];
}

/**
 * get load 3 overcurrent reconnect interval time
 * 
 * @return  overcurrent reconnect interval time value in ms
*/
uint16_t LoadParameter::getOvercurrentReconnectInterval3()
{
    return _shadowRegisters[24];
}

/**
 * get output mode 3
 * 
 * @return  output mode, 1 = source, 0 = sink
*/
uint16_t LoadParameter::getOutputMode3()
{
    return _shadowRegisters[25];
}

/**
 * get all parameter
 * 
 * @param[in]   regs array
 * 
 * @return number of written element
 */
size_t LoadParameter::getAllParameter(loadParamRegister &regs)
{
    size_t paramNumber = 0;
    for (size_t i = 0; i < _shadowRegisters.size(); i++)
    {
        regs[i] = _shadowRegisters[i];
        paramNumber++;
    }    
    return paramNumber;
}


/**
 * ============================================================
 */

/**
 * save baudrate into flash
 * 
 * @param[in]   value   baudrate
 */
void LoadParameter::setBaudrate(uint16_t value)
{
    if (value < 0 || value > 6)
    {
        return;
    }
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_baud", value);
    preferences.end();
    ESP_LOGI(_TAG, "set baudrate to %d\n", value);
}

/**
 * save id into flash
 * 
 * @param[in]   value   id
 */
void LoadParameter::setId(uint16_t value)
{
    if (value < 1 || value > 254)
    {
        return;
    }
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_id", value);
    preferences.end();
    ESP_LOGI(_TAG, "set id to %d\n", value);
}

/**
 * save overvoltage disconnect 1 into flash
 * 
 * @param[in]   value   overvoltage disconnect in 0.1V
 */
void LoadParameter::setOvervoltageDisconnect1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_d1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov dc 1 to %d\n", value);
}

/**
 * save overvoltage reconnect 1 into flash
 * 
 * @param[in]   value   overvoltage reconnect in 0.1V
 */
void LoadParameter::setOvervoltageReconnect1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_r1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov rc 1 to %d\n", value);
}

/**
 * save undervoltage disconnect 1 into flash
 * 
 * @param[in]   value   undervoltage disconnect in 0.1V
 */
void LoadParameter::setUndervoltageDisconnect1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_d1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv dc 1 to %d\n", value);
}

/**
 * save undervoltage reconnect 1 into flash
 * 
 * @param[in]   value   undervoltage reconnect in 0.1V
 */
void LoadParameter::setUndervoltageReconnect1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_r1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv rc 1 to %d\n", value);
}

/**
 * save overcurrent disconnect 1 into flash
 * 
 * @param[in]   value   overcurrent disconnect in 0.01A
 */
void LoadParameter::setOvercurrentDisconnect1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_d1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dc 1 to %d\n", value);
}

/**
 * save overcurrent detection time 1 into flash
 * 
 * @param[in]   value   overcurrent detection time in ms
 */
void LoadParameter::setOvercurrentDetectionTime1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_dt1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dt 1 to %d\n", value);
}

/**
 * save overcurrent reconnect interval 1 into flash
 * 
 * @param[in]   value   overcurrent reconnect interval time in ms
 */
void LoadParameter::setOvercurrentReconnectInterval1(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_rt1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc rt 1 to %d\n", value);
}

/**
 * save output mode 1 into flash
 * 
 * @param[in]   value   output mode (0 - 1)
 */
void LoadParameter::setOutputMode1(uint16_t value)
{
    if (value > 1)
    {
        return;
    }
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_om_1", value);
    preferences.end();
    ESP_LOGI(_TAG, "set om 1 to %d\n", value);
}

/**
 * ============================================================
 */

/**
 * save overvoltage disconnect 2 into flash
 * 
 * @param[in]   value   overvoltage disconnect in 0.1V
 */
void LoadParameter::setOvervoltageDisconnect2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_d2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov dc 2 to %d\n", value);
}

/**
 * save overvoltage reconnect 2 into flash
 * 
 * @param[in]   value   overvoltage reconnect in 0.1V
 */
void LoadParameter::setOvervoltageReconnect2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_r2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov rc 2 to %d\n", value);
}

/**
 * save undervoltage disconnect 2 into flash
 * 
 * @param[in]   value   undervoltage disconnect in 0.1V
 */
void LoadParameter::setUndervoltageDisconnect2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_d2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv dc 2 to %d\n", value);
}

/**
 * save undervoltage reconnect 2 into flash
 * 
 * @param[in]   value   undervoltage reconnect in 0.1V
 */
void LoadParameter::setUndervoltageReconnect2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_r2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv rc 2 to %d\n", value);
}

/**
 * save overcurrent disconnect 2 into flash
 * 
 * @param[in]   value   overcurrent disconnect in 0.01A
 */
void LoadParameter::setOvercurrentDisconnect2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_d2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dc 2 to %d\n", value);
}

/**
 * save overcurrent detection time 2 into flash
 * 
 * @param[in]   value   overcurrent detection time in ms
 */
void LoadParameter::setOvercurrentDetectionTime2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_dt2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dt 2 to %d\n", value);
}

/**
 * save overcurrent reconnect interval 2 into flash
 * 
 * @param[in]   value   overcurrent reconnect interval time in ms
 */
void LoadParameter::setOvercurrentReconnectInterval2(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_rt2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc rt 2 to %d\n", value);
}

/**
 * save output mode 2 into flash
 * 
 * @param[in]   value   output mode (0 - 1)
 */
void LoadParameter::setOutputMode2(uint16_t value)
{
    if (value > 1)
    {
        return;
    }
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_om_2", value);
    preferences.end();
    ESP_LOGI(_TAG, "set om 2 to %d\n", value);
}

/**
 * ============================================================
 */

/**
 * save overvoltage disconnect 3 into flash
 * 
 * @param[in]   value   overvoltage disconnect in 0.1V
 */
void LoadParameter::setOvervoltageDisconnect3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_d3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov dc 3 to %d\n", value);
}

/**
 * save overvoltage reconnect 3 into flash
 * 
 * @param[in]   value   overvoltage reconnect in 0.1V
 */
void LoadParameter::setOvervoltageReconnect3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_ov_r3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set ov rc 3 to %d\n", value);
}

/**
 * save undervoltage disconnect 3 into flash
 * 
 * @param[in]   value   undervoltage disconnect in 0.1V
 */
void LoadParameter::setUndervoltageDisconnect3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_d3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv dc 3 to %d\n", value);
}

/**
 * save undervoltage reconnect 3 into flash
 * 
 * @param[in]   value   undervoltage reconnect in 0.1V
 */
void LoadParameter::setUndervoltageReconnect3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_uv_r3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set uv rc 3 to %d\n", value);
}

/**
 * save overcurrent disconnect 3 into flash
 * 
 * @param[in]   value   overcurrent disconnect in 0.01A
 */
void LoadParameter::setOvercurrentDisconnect3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_d3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dc 3 to %d\n", value);
}

/**
 * save overcurrent detection time 3 into flash
 * 
 * @param[in]   value   overcurrent detection time in ms
 */
void LoadParameter::setOvercurrentDetectionTime3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_dt3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc dt 3 to %d\n", value);
}

/**
 * save overcurrent reconnect interval 3 into flash
 * 
 * @param[in]   value   overcurrent reconnect interval time in ms
 */
void LoadParameter::setOvercurrentReconnectInterval3(uint16_t value)
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_oc_rt3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set oc rt 3 to %d\n", value);
}

/**
 * save output mode 3 into flash
 * 
 * @param[in]   value   output mode (0 - 1)
 */
void LoadParameter::setOutputMode3(uint16_t value)
{
    if (value > 1)
    {
        return;
    }
    Preferences preferences;
    preferences.begin(_name.c_str());
    preferences.putUShort("u_om_3", value);
    preferences.end();
    ESP_LOGI(_TAG, "set om 3 to %d\n", value);
}

/**
 * print default parameter
*/
void LoadParameter::printDefault()
{
    Preferences preferences;
    preferences.begin(_name.c_str());

    ESP_LOGI(_TAG, "d_baud : %d\n", preferences.getUShort("d_baud"));
    ESP_LOGI(_TAG, "d_id : %d\n", preferences.getUShort("d_id"));

    ESP_LOGI(_TAG, "d_ov_d1 : %d\n", preferences.getUShort("d_ov_d1"));
    ESP_LOGI(_TAG, "d_ov_r1 : %d\n", preferences.getUShort("d_ov_r1"));
    ESP_LOGI(_TAG, "d_uv_d1 : %d\n", preferences.getUShort("d_uv_d1"));
    ESP_LOGI(_TAG, "d_uv_r1 : %d\n", preferences.getUShort("d_uv_r1"));
    ESP_LOGI(_TAG, "d_oc_d1 : %d\n", preferences.getUShort("d_oc_d1"));
    ESP_LOGI(_TAG, "d_oc_dt1 : %d\n", preferences.getUShort("d_oc_dt1"));
    ESP_LOGI(_TAG, "d_oc_rt1 : %d\n", preferences.getUShort("d_oc_rt1"));
    ESP_LOGI(_TAG, "d_om_1 : %d\n", preferences.getUShort("d_om_1"));

    ESP_LOGI(_TAG, "d_ov_d2 : %d\n", preferences.getUShort("d_ov_d2"));
    ESP_LOGI(_TAG, "d_ov_r2 : %d\n", preferences.getUShort("d_ov_r2"));
    ESP_LOGI(_TAG, "d_uv_d2 : %d\n", preferences.getUShort("d_uv_d2"));
    ESP_LOGI(_TAG, "d_uv_r2 : %d\n", preferences.getUShort("d_uv_r2"));
    ESP_LOGI(_TAG, "d_oc_d2 : %d\n", preferences.getUShort("d_oc_d2"));
    ESP_LOGI(_TAG, "d_oc_dt2 : %d\n", preferences.getUShort("d_oc_dt2"));
    ESP_LOGI(_TAG, "d_oc_rt2 : %d\n", preferences.getUShort("d_oc_rt2"));
    ESP_LOGI(_TAG, "d_om_2 : %d\n", preferences.getUShort("d_om_2"));

    ESP_LOGI(_TAG, "d_ov_d3 : %d\n", preferences.getUShort("d_ov_d3"));
    ESP_LOGI(_TAG, "d_ov_r3 : %d\n", preferences.getUShort("d_ov_r3"));
    ESP_LOGI(_TAG, "d_uv_d3 : %d\n", preferences.getUShort("d_uv_d3"));
    ESP_LOGI(_TAG, "d_uv_r3 : %d\n", preferences.getUShort("d_uv_r3"));
    ESP_LOGI(_TAG, "d_oc_d3 : %d\n", preferences.getUShort("d_oc_d3"));
    ESP_LOGI(_TAG, "d_oc_dt3 : %d\n", preferences.getUShort("d_oc_dt3"));
    ESP_LOGI(_TAG, "d_oc_rt3 : %d\n", preferences.getUShort("d_oc_rt3"));
    ESP_LOGI(_TAG, "d_om_3 : %d\n", preferences.getUShort("d_om_3"));

    preferences.end();
}

/**
 * print user parameter
*/
void LoadParameter::printUser()
{
    Preferences preferences;
    preferences.begin(_name.c_str());
    
    ESP_LOGI(_TAG, "u_baud : %d\n", preferences.getUShort("u_baud"));
    ESP_LOGI(_TAG, "u_id : %d\n", preferences.getUShort("u_id"));

    ESP_LOGI(_TAG, "u_ov_d1 : %d\n", preferences.getUShort("u_ov_d1"));
    ESP_LOGI(_TAG, "u_ov_r1 : %d\n", preferences.getUShort("u_ov_r1"));
    ESP_LOGI(_TAG, "u_uv_d1 : %d\n", preferences.getUShort("u_uv_d1"));
    ESP_LOGI(_TAG, "u_uv_r1 : %d\n", preferences.getUShort("u_uv_r1"));
    ESP_LOGI(_TAG, "u_oc_d1 : %d\n", preferences.getUShort("u_oc_d1"));
    ESP_LOGI(_TAG, "u_oc_dt1 : %d\n", preferences.getUShort("u_oc_dt1"));
    ESP_LOGI(_TAG, "u_oc_rt1 : %d\n", preferences.getUShort("u_oc_rt1"));
    ESP_LOGI(_TAG, "u_om_1 : %d\n", preferences.getUShort("u_om_1"));

    ESP_LOGI(_TAG, "u_ov_d2 : %d\n", preferences.getUShort("u_ov_d2"));
    ESP_LOGI(_TAG, "u_ov_r2 : %d\n", preferences.getUShort("u_ov_r2"));
    ESP_LOGI(_TAG, "u_uv_d2 : %d\n", preferences.getUShort("u_uv_d2"));
    ESP_LOGI(_TAG, "u_uv_r2 : %d\n", preferences.getUShort("u_uv_r2"));
    ESP_LOGI(_TAG, "u_oc_d2 : %d\n", preferences.getUShort("u_oc_d2"));
    ESP_LOGI(_TAG, "u_oc_dt2 : %d\n", preferences.getUShort("u_oc_dt2"));
    ESP_LOGI(_TAG, "u_oc_rt2 : %d\n", preferences.getUShort("u_oc_rt2"));
    ESP_LOGI(_TAG, "u_om_2 : %d\n", preferences.getUShort("u_om_2"));

    ESP_LOGI(_TAG, "u_ov_d3 : %d\n", preferences.getUShort("u_ov_d3"));
    ESP_LOGI(_TAG, "u_ov_r3 : %d\n", preferences.getUShort("u_ov_r3"));
    ESP_LOGI(_TAG, "u_uv_d3 : %d\n", preferences.getUShort("u_uv_d3"));
    ESP_LOGI(_TAG, "u_uv_r3 : %d\n", preferences.getUShort("u_uv_r3"));
    ESP_LOGI(_TAG, "u_oc_d3 : %d\n", preferences.getUShort("u_oc_d3"));
    ESP_LOGI(_TAG, "u_oc_dt3 : %d\n", preferences.getUShort("u_oc_dt3"));
    ESP_LOGI(_TAG, "u_oc_rt3 : %d\n", preferences.getUShort("u_oc_rt3"));
    ESP_LOGI(_TAG, "u_om_3 : %d\n", preferences.getUShort("u_om_3"));

    preferences.end();
}

void LoadParameter::printShadow()
{
    for (size_t i = 0; i < _shadowRegisters.size(); i++)
    {
        ESP_LOGI(_TAG, "value %d = %d\n", i, _shadowRegisters[i]);
    }
}

LoadParameter::~LoadParameter()
{
}