#pragma once
// AS: papaGame.data.customers.CustomerPrudence
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerPrudence : public CustomerDataFile {
public:
    CustomerPrudence() { CustomerPrudence::initCustomerData(); }

    // AS: CustomerPrudence.initCustomerData
    void initCustomerData() override
    {
        customerName = "Prudence";
        customerClipName = "Prudence";
        customerFirstGame = "Papa Louie";
        weaponName = "Dog Squeak Toys";
        weaponClipName = "dogtoy";
        bulletName = "DogToy";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_NONE;
        trappedWorld = 1;
        trappedChar = 1;
        toss_offset_extra_x = 0;
        toss_offset_extra_y = 15;
    }
};

} // namespace papaGame::data::customers
