#pragma once
// AS: papaGame.data.customers.CustomerMindy
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerMindy : public CustomerDataFile {
public:
    CustomerMindy() { CustomerMindy::initCustomerData(); }

    // AS: CustomerMindy.initCustomerData
    void initCustomerData() override
    {
        customerName = "Mindy";
        customerClipName = "Mindy";
        customerFirstGame = "Papa Louie";
        weaponName = "Hair Dryer Whip";
        weaponClipName = "hairdryer";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_NONE;
        trappedWorld = 2;
        trappedChar = 2;
        jumpstart = -25;
        skiddecayamount = 0.7;
    }
};

} // namespace papaGame::data::customers
