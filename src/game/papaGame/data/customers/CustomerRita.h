#pragma once
// AS: papaGame.data.customers.CustomerRita
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerRita : public CustomerDataFile {
public:
    CustomerRita() { CustomerRita::initCustomerData(); }

    // AS: CustomerRita.initCustomerData
    void initCustomerData() override
    {
        customerName = "Rita";
        customerClipName = "Rita";
        customerFirstGame = "Papa Louie";
        weaponName = "Spatula";
        weaponClipName = "spatula";
        weaponType = CustomerData::WEAPON_SWING1;
        skillType = CustomerData::SKILL_NONE;
    }
};

} // namespace papaGame::data::customers
