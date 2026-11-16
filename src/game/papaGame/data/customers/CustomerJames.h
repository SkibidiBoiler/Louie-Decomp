#pragma once
// AS: papaGame.data.customers.CustomerJames
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerJames : public CustomerDataFile {
public:
    CustomerJames() { CustomerJames::initCustomerData(); }

    // AS: CustomerJames.initCustomerData
    void initCustomerData() override
    {
        customerName = "James";
        customerClipName = "James";
        customerFirstGame = "Papa Louie";
        weaponName = "Plunger";
        weaponClipName = "plunger";
        weaponType = CustomerData::WEAPON_SWING1;
        skillType = CustomerData::SKILL_PUSH;
        trappedWorld = 6;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
