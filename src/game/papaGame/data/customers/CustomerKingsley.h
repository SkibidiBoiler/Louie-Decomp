#pragma once
// AS: papaGame.data.customers.CustomerKingsley
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerKingsley : public CustomerDataFile {
public:
    CustomerKingsley() { CustomerKingsley::initCustomerData(); }

    // AS: CustomerKingsley.initCustomerData
    void initCustomerData() override
    {
        customerName = "Kingsley";
        customerClipName = "Kingsley";
        customerFirstGame = "Papa's Pizzeria";
        weaponName = "Microphone";
        weaponClipName = "microphone";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_POUND;
        isHuge = true;
        trappedWorld = 5;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
