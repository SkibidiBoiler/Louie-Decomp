#pragma once
// AS: papaGame.data.customers.CustomerZoe
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerZoe : public CustomerDataFile {
public:
    CustomerZoe() { CustomerZoe::initCustomerData(); }

    // AS: CustomerZoe.initCustomerData
    void initCustomerData() override
    {
        customerName = "Zoe";
        customerClipName = "Zoe";
        customerFirstGame = "Papa's Taco Mia!";
        weaponName = "Guitar";
        weaponClipName = "guitar";
        weaponType = CustomerData::WEAPON_SWING2;
        skillType = CustomerData::SKILL_NONE;
        weaponHasStyles = true;
        trappedWorld = 8;
        trappedChar = 3;
        skiddecayamount = 1.5;
        jumpstart = -24;
    }
};

} // namespace papaGame::data::customers
