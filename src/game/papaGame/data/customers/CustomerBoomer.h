#pragma once
// AS: papaGame.data.customers.CustomerBoomer
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerBoomer : public CustomerDataFile {
public:
    CustomerBoomer() { CustomerBoomer::initCustomerData(); }

    // AS: CustomerBoomer.initCustomerData
    void initCustomerData() override
    {
        customerName = "Boomer";
        customerClipName = "Boomer";
        customerFirstGame = "Papa's Wingeria";
        weaponName = "Roman Candle";
        weaponClipName = "romancandle";
        bulletName = "RomanCandle";
        weaponType = CustomerData::WEAPON_PISTOL;
        skillType = CustomerData::SKILL_GLIDE;
        bulletstart = 2;
        trappedWorld = 3;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
