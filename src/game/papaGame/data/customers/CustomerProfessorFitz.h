#pragma once
// AS: papaGame.data.customers.CustomerProfessorFitz
#include "game/papaGame/data/CustomerData.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace papaGame::data::customers {

class CustomerProfessorFitz : public CustomerDataFile {
public:
    CustomerProfessorFitz() { CustomerProfessorFitz::initCustomerData(); }

    // AS: CustomerProfessorFitz.initCustomerData
    void initCustomerData() override
    {
        customerName = "Professor Fitz";
        customerClipName = "ProfessorFitz";
        customerFirstGame = "Papa's Wingeria";
        weaponName = "Beaker Bombs";
        weaponClipName = "beaker";
        bulletName = "Beaker";
        weaponType = CustomerData::WEAPON_TOSS;
        skillType = CustomerData::SKILL_GLIDE;
        trappedWorld = 3;
        trappedChar = 3;
        toss_offset_extra_x = -15;
    }
};

} // namespace papaGame::data::customers
