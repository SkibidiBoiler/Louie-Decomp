#include "game/papaGame/data/CustomerData.h"

#include "game/as3.h"
#include "game/papaGame/data/customers/CustomerMarty.h"
#include "game/papaGame/data/customers/CustomerRita.h"

namespace papaGame::data {

// AS: CustomerData.CustomerData
CustomerData::CustomerData(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupCustomerData();
}

// AS: CustomerData.setupCustomerData
void CustomerData::setupCustomerData()
{
    // customersIncluded = [CustomerMarty, CustomerRita, CustomerPrudence, CustomerTaylor, ... CustomerXandra]
    // SKELETON: only the two starting customers are translated so far; the order of the rest must be kept.
    customerDataFiles.clear();
    customerDataFiles.push_back(std::make_unique<customers::CustomerMarty>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerRita>());
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
// SKELETON: only Marty and Rita are in the port's customer list so far, and neither is ever trapped.
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
