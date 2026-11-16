#pragma once
// AS: papaGame.data.customers.CustomerTaylor
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerTaylor : public CustomerDataFile {
public:
    CustomerTaylor() { CustomerTaylor::initCustomerData(); }

    // AS: CustomerTaylor.initCustomerData
    void initCustomerData() override
    {
        customerName = "Taylor";
        customerClipName = "Taylor";
        customerFirstGame = "Papa Louie";
        weaponName = "Ear Buds";
        weaponClipName = "headphones";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_NONE;
        trappedWorld = 1;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
