#pragma once
// AS: papaGame.data.customers.CustomerKahuna
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerKahuna : public CustomerDataFile {
public:
    CustomerKahuna() { CustomerKahuna::initCustomerData(); }

    // AS: CustomerKahuna.initCustomerData
    void initCustomerData() override
    {
        customerName = "Kahuna";
        customerClipName = "Kahuna";
        customerFirstGame = "Papa's Freezeria";
        weaponName = "Surfboard";
        weaponClipName = "surfboard";
        weaponType = CustomerData::WEAPON_KAHUNA;
        skillType = CustomerData::SKILL_POUND;
        isHuge = true;
        trappedWorld = 3;
        trappedChar = 2;
    }
};

} // namespace papaGame::data::customers
