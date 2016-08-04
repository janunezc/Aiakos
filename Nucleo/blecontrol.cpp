#include "blecontrol.h"
#include "rn4020.h"
#include "btcharacteristic.h"

extern SoftwareSerial sw;
rn4020 rn(Serial,5,A3,A4,6);
SoftwareSerial* sPortDebug;
static void connectionEvent(bool bConnectionUp);
static void alertLevelEvent(char* value);

//https://www.bluetooth.com/specifications/gatt/services
//https://www.bluetooth.com/specifications/gatt/characteristics
static btCharacteristic ias_alertLevel("1802", "2A06", btCharacteristic::WRITE_WOUT_RESP, 1, btCharacteristic::NOTHING);
//Private UUIDs have been generated by: https://www.uuidgenerator.net/version4
static btCharacteristic rfid_key("f1a87912-5950-479c-a5e5-b6cc81cd0502",
                          "855b1938-83e2-4889-80b7-ae58fcd0e6ca",
                          btCharacteristic::WRITE_WOUT_RESP,5,
                          btCharacteristic::ENCR_W);

bleControl::bleControl()
{
    ias_alertLevel.setListener(alertLevelEvent);
}

bool bleControl::begin(bool bCentral)
{
    if(!rn.begin(2400))
    {
        //Maybe the module is blocked or set to an unknown baudrate?
        if(!rn.doFactoryDefault())
        {
            return false;
        }
        //Factory default baud=115200
        if(!rn.begin(115200))
        {
            return false;
        }
        //Switch to 2400baud
        // + It's more reliable than 115200baud with the ProTrinket 3V.
        // + It also works when the module is in deep sleep mode.
        if(!rn.setBaudrate(2400))
        {
            return false;
        }
        //Baudrate only becomes active after resetting the module.
        if(!rn.doReboot(2400))
        {
            return false;
        }
    }
    rn.setConnectionListener(connectionEvent);
    if(bCentral)
    {
        //Central
        //    ble2_reset_to_factory_default(RESET_SOME);
        //    ble2_set_server_services(0xC0000000);
        //    ble2_set_supported_features(0x82480000);
        //    ble2_device_reboot();
    }else
    {
        //Peripheral
        if(!rn.setRole(rn4020::PERIPHERAL))
        {
            return false;
        }
        if(!rn.setTxPower(0))
        {
            return false;
        }
        if(!rn.removePrivateCharacteristics())
        {
            return false;
        }
        //Power must be cycled after removing private characteristics
        if(!rn.begin(2400))
        {
            return false;
        }
        if(!rn.addCharacteristic(&rfid_key))
        {
            return false;
        }
        if(!rn.addCharacteristic(&ias_alertLevel))
        {
            return false;
        }
        //Reboot needed to make above settings take effect
        if(!rn.doReboot(2400))
        {
            return false;
        }
        //Start advertizing to make the RN4020 discoverable & connectable
        if(!rn.doAdvertizing(true,5000))
        {
            return false;
        }
        //If the following statement is commented out, the addCharacteristic function will no longer work.
        if(!rn.setOperatingMode(rn4020::DEEP_SLEEP))
        {
            return false;
        }
    }
    return true;
}

bool bleControl::loop()
{
    rn.loop();
}

void connectionEvent(bool bConnectionUp)
{
    if(bConnectionUp)
    {
        sw.println("up");
    }else
    {
        sw.println("down");
        //After connection went down, advertizing must be restarted or the module will no longer be connectable.
        if(!rn.doAdvertizing(true,5000))
        {
            return;
        }
    }
}

void alertLevelEvent(char* value)
{
    sw.println(value);
}

