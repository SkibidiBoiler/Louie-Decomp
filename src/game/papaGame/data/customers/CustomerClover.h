#pragma once
// AS: papaGame.data.customers.CustomerClover
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerClover : public CustomerDataFile {
public:
    CustomerClover() { CustomerClover::initCustomerData(); }

    // AS: CustomerClover.initCustomerData
    void initCustomerData() override
    {
        customerName = "Clover";
        customerClipName = "Clover";
        customerFirstGame = "Papa Louie";
        weaponName = "Drumsticks";
        weaponClipName = "clover";
        weaponType = CustomerData::WEAPON_MELEE;
        skillType = CustomerData::SKILL_NONE;
        trappedWorld = 1;
        trappedChar = 3;
    }
};

} // namespace papaGame::data::customers
