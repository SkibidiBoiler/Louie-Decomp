#pragma once
// AS: papaGame.data.CustomerDataFile — one playable customer: clip names, weapon, skill and movement values.
#include <string>

namespace papaGame::data {

class CustomerDataFile {
public:
    virtual ~CustomerDataFile() = default;

    // AS: CustomerDataFile.initCustomerData (the AS constructor calls it; in C++ each subclass constructor
    // calls its own override, since a base constructor cannot reach it).
    virtual void initCustomerData() {}

    std::string customerName = "Placeholder";
    std::string customerClipName = "Placeholder";
    std::string customerFirstGame = "Papa's Pizzeria";
    std::string thumbColor = "lime";
    std::string weaponName = "Weapon Name";
    std::string weaponClipName = "WeaponName";
    std::string bulletName = "BulletName";
    std::string weaponType = "melee";
    std::string skillType = "none";
    bool weaponHasStyles = false;
    bool gliderHasStyles = false;
    bool isHuge = false;
    double trappedWorld = -1;
    double trappedChar = -1;
    double jumpstart = -23;
    double jumpgravity = 2;
    double walkspeed = 8;
    double maxskidspeed = 8;
    double skiddecay = 1;
    double skiddecayamount = 1;
    double bulletstart = 5;
    double toss_offset_extra_x = 0;
    double toss_offset_extra_y = 0;
};

} // namespace papaGame::data
