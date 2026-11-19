#pragma once
// AS: papaGame.data.customers.CustomerMarty
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerMarty : public CustomerDataFile {
public:
    CustomerMarty() { CustomerMarty::initCustomerData(); }

    // AS: CustomerMarty.initCustomerData
    void initCustomerData() override
    {
        customerName = "Marty";
        customerClipName = "Marty";
        customerFirstGame = "Papa Louie";
        weaponName = "Spatula";
        weaponClipName = "spatula";
        weaponType = CustomerData::WEAPON_SWING1;
        skillType = CustomerData::SKILL_NONE;
    }
};

} // namespace papaGame::data::customers
