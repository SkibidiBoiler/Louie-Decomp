#pragma once
// AS: papaGame.data.CustomerData — the list of playable customers and lookups by index.
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"
#include "game/papaGame/data/CustomerDataFile.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::data {

class CustomerData {
public:
    static constexpr const char *SKILL_NONE = "none";
    static constexpr const char *SKILL_POUND = "pound";
    static constexpr const char *SKILL_GLIDE = "glide";
    static constexpr const char *SKILL_PUSH = "push";
    static constexpr const char *SKILL_DOUBLEJUMP = "doublejump";
    static constexpr const char *SKILL_WALLJUMP = "walljump";
    static constexpr const char *SKILL_CRAWL = "crawl";
    static constexpr const char *WEAPON_SWING1 = "swing1";
    static constexpr const char *WEAPON_SWING2 = "swing2";
    static constexpr const char *WEAPON_KAHUNA = "kahuna";
    static constexpr const char *WEAPON_SCOOTER = "scooter";
    static constexpr const char *WEAPON_MELEE = "melee";
    static constexpr const char *WEAPON_WHIP = "whip";
    static constexpr const char *WEAPON_TOSS = "toss";
    static constexpr const char *WEAPON_PISTOL = "pistol";
    static constexpr const char *WEAPON_LONGGUN = "longgun";
    static constexpr const char *WEAPON_BAZOOKA = "bazooka";

    explicit CustomerData(obf_9G::obf_4R *gameObj);

    void setupCustomerData();
    std::string getCustomerClipName(double index) const;
    std::string getCustomerType(double index) const;
    std::string getCustomerSkill(double index) const;
    std::string getWeaponClipName(double index, double style) const;
    const CustomerDataFile *getCustomerData(double index) const;
    double getTrappedCustomerIndex(double whichWorld, double cageIndex) const;
    std::string getCustomerName(double index) const; // AS: CustomerData.getCustomerName
    // AS: CustomerData.generateCustomerBitmap — every customer's head, 45 x 45 each, in a row (thumbBMP). The port
    // draws it into a texture at kThumbScale. SKELETON: the DropShadowFilter.
    void generateCustomerBitmap();
    // AS: CustomerData.getCustomerBitmap — that customer's cell, as a Bitmap child (45 x 45 at the origin).
    engine::MovieClip::BitmapChild getCustomerBitmap(double index) const;
    ~CustomerData();

    obf_9G::obf_4R *gameObj = nullptr;
    static constexpr float kThumbScale = 2;

private:
    std::vector<std::unique_ptr<CustomerDataFile>> customerDataFiles;
    SDL_Texture *thumbBMP = nullptr;
};

} // namespace papaGame::data
