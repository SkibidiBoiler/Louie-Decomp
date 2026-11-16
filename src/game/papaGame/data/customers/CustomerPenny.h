#pragma once
// AS: papaGame.data.customers.CustomerPenny
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerPenny : public CustomerDataFile {
public:
    CustomerPenny() { CustomerPenny::initCustomerData(); }

    // AS: CustomerPenny.initCustomerData
    void initCustomerData() override
    {
        customerName = "Penny";
        customerClipName = "Penny";
        customerFirstGame = "Papa Louie";
        weaponName = "Blueberry Shooter";
        weaponClipName = "blueberryshooter";
        bulletName = "BlueberryShot";
        weaponType = CustomerData::WEAPON_LONGGUN;
        skillType = CustomerData::SKILL_WALLJUMP;
        bulletstart = 2;
        trappedWorld = 7;
        trappedChar = 3;
    }
};

} // namespace papaGame::data::customers
