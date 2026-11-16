#pragma once
// AS: papaGame.data.customers.CustomerXandra
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerXandra : public CustomerDataFile {
public:
    CustomerXandra() { CustomerXandra::initCustomerData(); }

    // AS: CustomerXandra.initCustomerData
    void initCustomerData() override
    {
        customerName = "Xandra";
        customerClipName = "Xandra";
        customerFirstGame = "Papa's Taco Mia!";
        weaponName = "X-Wand";
        weaponClipName = "xwand";
        bulletName = "XandraStar";
        bulletstart = 3;
        weaponType = CustomerData::WEAPON_SWING1;
        skillType = CustomerData::SKILL_GLIDE;
        gliderHasStyles = true;
        trappedWorld = 10;
        trappedChar = 1;
        jumpstart = -19;
        jumpgravity = 1;
        walkspeed = 7;
        maxskidspeed = 7;
        skiddecayamount = 0.7;
    }
};

} // namespace papaGame::data::customers
