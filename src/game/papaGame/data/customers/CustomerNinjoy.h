#pragma once
// AS: papaGame.data.customers.CustomerNinjoy
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerNinjoy : public CustomerDataFile {
public:
    CustomerNinjoy() { CustomerNinjoy::initCustomerData(); }

    // AS: CustomerNinjoy.initCustomerData
    void initCustomerData() override
    {
        customerName = "Ninjoy";
        customerClipName = "Ninjoy";
        customerFirstGame = "Papa's Freezeria";
        weaponName = "Karate";
        weaponClipName = "ninjoy";
        weaponType = CustomerData::WEAPON_MELEE;
        skillType = CustomerData::SKILL_WALLJUMP;
        jumpstart = -25;
        walkspeed = 9;
        trappedWorld = 7;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
