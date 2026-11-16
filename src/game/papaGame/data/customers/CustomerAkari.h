#pragma once
// AS: papaGame.data.customers.CustomerAkari
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerAkari : public CustomerDataFile {
public:
    CustomerAkari() { CustomerAkari::initCustomerData(); }

    // AS: CustomerAkari.initCustomerData
    void initCustomerData() override
    {
        customerName = "Akari";
        customerClipName = "Akari";
        customerFirstGame = "Papa's Burgeria";
        weaponName = "Bananazooka";
        weaponClipName = "bananazooka";
        bulletName = "Banana";
        weaponType = CustomerData::WEAPON_BAZOOKA;
        skillType = CustomerData::SKILL_NONE;
        bulletstart = 5;
        trappedWorld = 2;
        trappedChar = 3;
    }
};

} // namespace papaGame::data::customers
