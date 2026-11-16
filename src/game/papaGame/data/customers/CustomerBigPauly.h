#pragma once
// AS: papaGame.data.customers.CustomerBigPauly
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerBigPauly : public CustomerDataFile {
public:
    CustomerBigPauly() { CustomerBigPauly::initCustomerData(); }

    // AS: CustomerBigPauly.initCustomerData
    void initCustomerData() override
    {
        customerName = "Big Pauly";
        customerClipName = "BigPauly";
        customerFirstGame = "Papa Louie";
        weaponName = "Pepper Bombs";
        weaponClipName = "pepperjar";
        bulletName = "PepperBomb";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_POUND;
        isHuge = true;
        trappedWorld = 2;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
