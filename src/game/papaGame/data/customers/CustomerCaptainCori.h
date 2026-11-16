#pragma once
// AS: papaGame.data.customers.CustomerCaptainCori
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerCaptainCori : public CustomerDataFile {
public:
    CustomerCaptainCori() { CustomerCaptainCori::initCustomerData(); }

    // AS: CustomerCaptainCori.initCustomerData
    void initCustomerData() override
    {
        customerName = "Captain Cori";
        customerClipName = "CaptainCori";
        customerFirstGame = "Papa's Freezeria";
        weaponName = "Anchor";
        weaponClipName = "anchorwhip";
        weaponType = CustomerData::WEAPON_WHIP;
        skillType = CustomerData::SKILL_PUSH;
        trappedWorld = 6;
        trappedChar = 3;
    }
};

} // namespace papaGame::data::customers
