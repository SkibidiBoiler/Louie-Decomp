#pragma once
// AS: papaGame.data.customers.CustomerConnor
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerConnor : public CustomerDataFile {
public:
    CustomerConnor() { CustomerConnor::initCustomerData(); }

    // AS: CustomerConnor.initCustomerData
    void initCustomerData() override
    {
        customerName = "Connor";
        customerClipName = "Connor";
        customerFirstGame = "Papa's Burgeria";
        weaponName = "Basketball";
        weaponClipName = "basketball";
        bulletName = "Basketball";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_DOUBLEJUMP;
        trappedWorld = 5;
        trappedChar = 3;
    }
};

} // namespace papaGame::data::customers
