#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
std::string pending;
int visible=0;
bool damage_on=true, items_on=true, planets_on=true;
void queue(std::string s){ pending=std::move(s); }
}
void load(openrac::Mod& mod){
    if(mod.game()!="rac1" || !mod.has_notice()) throw openrac::Error("Better HUD requires RAC1 interface 1.9+");
    g=&mod;
    damage_on=mod.setting<bool>("damage");
    items_on=mod.setting<bool>("items"); planets_on=mod.setting<bool>("planets");
    mod.hook<openrac::rac1::Hero_TakeDamage>([](auto original,std::int32_t amount){
        original(amount);
        if(damage_on){ openrac_pose p{}; p.size=sizeof p; if(g->hero_pose(p)) queue(qol::health(p.health)); }
    });
    mod.hook<openrac::rac1::Item_Give>([](auto original,std::int32_t id,std::int32_t equip){
        original(id,equip); if(items_on) queue("Item acquired: "+std::to_string(id));
    });
    mod.hook<openrac::rac1::Planet_Unlock>([](auto original,std::int32_t level){
        original(level); if(planets_on) queue("Planet unlocked: "+g->level_name(level));
    });
    mod.on_frame([]{
        if(!pending.empty()){ g->play_notice(pending); pending.clear(); visible=180; }
        else if(visible>0 && --visible==0) g->play_notice("");
    });
    mod.info("event-driven HUD notices");
}
OPENRAC_MOD_CPP("better-hud","0.1.1",load)
