#pragma once
// AS: papaGame.data.customers.CustomerPeggy
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerPeggy : public CustomerDataFile {
public:
    CustomerPeggy() { CustomerPeggy::initCustomerData(); }

    // AS: CustomerPeggy.initCustomerData
    void initCustomerData() override
    {
        customerName = "Peggy";
        customerClipName = "Peggy";
        customerFirstGame = "Papa's Pizzeria";
        weaponName = "Lasso";
        weaponClipName = "lassowhip";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_DOUBLEJUMP;
        trappedWorld = 7;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
