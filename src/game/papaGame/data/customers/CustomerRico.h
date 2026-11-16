#pragma once
// AS: papaGame.data.customers.CustomerRico
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerRico : public CustomerDataFile {
public:
    CustomerRico() { CustomerRico::initCustomerData(); }

    // AS: CustomerRico.initCustomerData
    void initCustomerData() override
    {
        customerName = "Rico";
        customerClipName = "Rico";
        customerFirstGame = "Papa's Taco Mia!";
        weaponName = "Chili Bombs";
        weaponClipName = "chilican";
        bulletName = "ChiliCan";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_PUSH;
        isHuge = true;
        trappedWorld = 8;
        trappedChar = 2;
        toss_offset_extra_x = -15;
    }
};

} // namespace papaGame::data::customers
