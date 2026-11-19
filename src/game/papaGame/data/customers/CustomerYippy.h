#pragma once
// AS: papaGame.data.customers.CustomerYippy
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerYippy : public CustomerDataFile {
public:
    CustomerYippy() { CustomerYippy::initCustomerData(); }

    // AS: CustomerYippy.initCustomerData
    void initCustomerData() override
    {
        customerName = "Yippy";
        customerClipName = "Yippy";
        customerFirstGame = "Papa's Pancakeria";
        weaponName = "Rainbow Cookies";
        weaponClipName = "cookie";
        bulletName = "Cookie";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_CRAWL;
        trappedWorld = 4;
        trappedChar = 3;
        toss_offset_extra_x = 10;
        toss_offset_extra_y = 10;
    }
};

} // namespace papaGame::data::customers
