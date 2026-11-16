#pragma once
// AS: papaGame.data.customers.CustomerFoodini
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerFoodini : public CustomerDataFile {
public:
    CustomerFoodini() { CustomerFoodini::initCustomerData(); }

    // AS: CustomerFoodini.initCustomerData
    void initCustomerData() override
    {
        customerName = "Foodini";
        customerClipName = "Foodini";
        customerFirstGame = "Papa's Pancakeria";
        weaponName = "Balloon Whip";
        weaponClipName = "balloonwhip";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_GLIDE;
        isHuge = true;
        trappedWorld = 4;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
