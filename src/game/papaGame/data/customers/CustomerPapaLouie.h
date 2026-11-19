#pragma once
// AS: papaGame.data.customers.CustomerPapaLouie
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerPapaLouie : public CustomerDataFile {
public:
    CustomerPapaLouie() { CustomerPapaLouie::initCustomerData(); }

    // AS: CustomerPapaLouie.initCustomerData
    void initCustomerData() override
    {
        customerName = "Papa Louie";
        customerClipName = "PapaLouie";
        customerFirstGame = "Papa Louie";
        weaponName = "Pizza Paddle";
        weaponClipName = "paddle";
        weaponType = CustomerData::WEAPON_SWING2;
        skillType = CustomerData::SKILL_GLIDE;
        weaponHasStyles = true;
        trappedWorld = 9;
        trappedChar = 1;
    }
};

} // namespace papaGame::data::customers
