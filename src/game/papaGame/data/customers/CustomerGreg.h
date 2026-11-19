#pragma once
// AS: papaGame.data.customers.CustomerGreg
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerGreg : public CustomerDataFile {
public:
    CustomerGreg() { CustomerGreg::initCustomerData(); }

    // AS: CustomerGreg.initCustomerData
    void initCustomerData() override
    {
        customerName = "Greg";
        customerClipName = "Greg";
        customerFirstGame = "Papa Louie";
        weaponName = "Yo-Yo";
        weaponClipName = "yoyo";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_CRAWL;
        trappedWorld = 6;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
