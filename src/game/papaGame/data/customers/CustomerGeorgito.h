#pragma once
// AS: papaGame.data.customers.CustomerGeorgito
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerGeorgito : public CustomerDataFile {
public:
    CustomerGeorgito() { CustomerGeorgito::initCustomerData(); }

    // AS: CustomerGeorgito.initCustomerData
    void initCustomerData() override
    {
        customerName = "Georgito";
        customerClipName = "Georgito";
        customerFirstGame = "Papa's Taco Mia!";
        weaponName = "Money Bag";
        weaponClipName = "moneybag";
        weaponType = CustomerData::WEAPON_SWING1;
        skillType = CustomerData::SKILL_CRAWL;
        trappedWorld = 4;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
