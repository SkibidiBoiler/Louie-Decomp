#pragma once
// AS: papaGame.data.customers.CustomerSargeFan
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerSargeFan : public CustomerDataFile {
public:
    CustomerSargeFan() { CustomerSargeFan::initCustomerData(); }

    // AS: CustomerSargeFan.initCustomerData
    void initCustomerData() override
    {
        customerName = "Sarge Fan";
        customerClipName = "SargeFan";
        customerFirstGame = "Papa's Pizzeria";
        weaponName = "Gummy Onions";
        weaponClipName = "gummyonion";
        bulletName = "GummyOnion";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_WALLJUMP;
        trappedWorld = 8;
        trappedChar = 1;
        toss_offset_extra_x = 0;
        toss_offset_extra_y = 15;
    }
};

} // namespace papaGame::data::customers
