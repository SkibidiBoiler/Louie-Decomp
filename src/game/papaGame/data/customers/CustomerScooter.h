#pragma once
// AS: papaGame.data.customers.CustomerScooter
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerScooter : public CustomerDataFile {
public:
    CustomerScooter() { CustomerScooter::initCustomerData(); }

    // AS: CustomerScooter.initCustomerData
    void initCustomerData() override
    {
        customerName = "Scooter";
        customerClipName = "Scooter";
        customerFirstGame = "Papa's Wingeria";
        weaponName = "Skateboard";
        weaponClipName = "skateboard";
        weaponType = CustomerData::WEAPON_SCOOTER;
        skillType = CustomerData::SKILL_DOUBLEJUMP;
        trappedWorld = 5;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
