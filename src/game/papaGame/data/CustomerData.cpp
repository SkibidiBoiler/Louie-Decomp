#include "game/papaGame/data/CustomerData.h"

#include "game/as3.h"
#include "game/papaGame/data/customers/CustomerAkari.h"
#include "game/papaGame/data/customers/CustomerBigPauly.h"
#include "game/papaGame/data/customers/CustomerBoomer.h"
#include "game/papaGame/data/customers/CustomerCaptainCori.h"
#include "game/papaGame/data/customers/CustomerClover.h"
#include "game/papaGame/data/customers/CustomerConnor.h"
#include "game/papaGame/data/customers/CustomerFoodini.h"
#include "game/papaGame/data/customers/CustomerGeorgito.h"
#include "game/papaGame/data/customers/CustomerGreg.h"
#include "game/papaGame/data/customers/CustomerJames.h"
#include "game/papaGame/data/customers/CustomerKahuna.h"
#include "game/papaGame/data/customers/CustomerKingsley.h"
#include "game/papaGame/data/customers/CustomerMarty.h"
#include "game/papaGame/data/customers/CustomerMindy.h"
#include "game/papaGame/data/customers/CustomerNinjoy.h"
#include "game/papaGame/data/customers/CustomerPapaLouie.h"
#include "game/papaGame/data/customers/CustomerPeggy.h"
#include "game/papaGame/data/customers/CustomerPenny.h"
#include "game/papaGame/data/customers/CustomerProfessorFitz.h"
#include "game/papaGame/data/customers/CustomerPrudence.h"
#include "game/papaGame/data/customers/CustomerRico.h"
#include "game/papaGame/data/customers/CustomerRita.h"
#include "game/papaGame/data/customers/CustomerSargeFan.h"
#include "game/papaGame/data/customers/CustomerScooter.h"
#include "game/papaGame/data/customers/CustomerTaylor.h"
#include "game/papaGame/data/customers/CustomerXandra.h"
#include "game/papaGame/data/customers/CustomerYippy.h"
#include "game/papaGame/data/customers/CustomerZoe.h"

namespace papaGame::data {

// AS: CustomerData.CustomerData
CustomerData::CustomerData(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupCustomerData();
}

// AS: CustomerData.setupCustomerData
void CustomerData::setupCustomerData()
{
    // customersIncluded, in the original's order (the index is the customer's id in the save data).
    customerDataFiles.clear();
    customerDataFiles.push_back(std::make_unique<customers::CustomerMarty>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerRita>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPrudence>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerTaylor>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerClover>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerMindy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerAkari>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerZoe>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerBigPauly>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerKahuna>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerKingsley>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerNinjoy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPenny>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerSargeFan>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerJames>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerScooter>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerConnor>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPeggy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerGeorgito>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerYippy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerGreg>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerCaptainCori>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerRico>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerBoomer>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerProfessorFitz>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerFoodini>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPapaLouie>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerXandra>());
}

// AS: CustomerData.getCustomerClipName
std::string CustomerData::getCustomerClipName(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->customerClipName : std::string();
}

// AS: CustomerData.getCustomerType
std::string CustomerData::getCustomerType(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->weaponType : std::string();
}

// AS: CustomerData.getCustomerSkill
std::string CustomerData::getCustomerSkill(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->skillType : std::string();
}

// AS: CustomerData.getWeaponClipName
std::string CustomerData::getWeaponClipName(double index, double style) const
{
    const CustomerDataFile *c = getCustomerData(index);
    if (!c) {
        return {};
    }
    std::string name = c->weaponClipName;
    if (c->weaponHasStyles && style > 1) {
        name += as3::String(style);
    }
    return name;
}

// AS: CustomerData.getCustomerData
const CustomerDataFile *CustomerData::getCustomerData(double index) const
{
    if (index >= 0 && index < static_cast<double>(customerDataFiles.size())) {
        return customerDataFiles[static_cast<size_t>(index)].get();
    }
    return nullptr;
}

// AS: CustomerData.getTrappedCustomerIndex — the customer caged in this world's cage `cageIndex`, or -1.
double CustomerData::getTrappedCustomerIndex(double whichWorld, double cageIndex) const
{
    for (size_t i = 0; i < customerDataFiles.size(); ++i) {
        const CustomerDataFile &c = *customerDataFiles[i];
        if (c.trappedWorld == whichWorld + 1 && c.trappedChar == cageIndex) {
            return static_cast<double>(i);
        }
    }
    return -1;
}

} // namespace papaGame::data
