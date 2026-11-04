#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
openrac::UiPage page;
std::string last_event="No recent HUD event";
std::uint16_t previous=0;
bool open_requested=false;
}

void load(openrac::Mod& mod){
    if(mod.game()!="rac1") throw openrac::Error("Better HUD is for RAC1");
    g=&mod;

    page=mod.ui_page([&mod](const openrac::UiPage& p,const openrac::UiEvent&){
        const auto status=mod.play_status();
        openrac_pose pose{}; pose.size=sizeof pose;
        const bool hero=mod.hero_pose(pose);
        openrac::UiContent c;
        c.title="Status";
        c.items={
            openrac::UiItem::text(status.level>=0 ? "Planet: "+mod.level_name(status.level) : "Planet: -"),
            openrac::UiItem::text("Health: "+std::to_string(hero?pose.health:0)),
            openrac::UiItem::text("Last event: "+last_event)
        };
        c.footer={"Native game UI only - no duplicate bolt display","L2 + R2 + Circle opens this page"};
        p.set(c);
    });

    mod.hook<openrac::rac1::Hero_TakeDamage>([](auto original,std::int32_t amount){
        original(amount);
        openrac_pose p{}; p.size=sizeof p;
        if(g->hero_pose(p)) last_event="Health changed to "+std::to_string(p.health);
    });
    mod.hook<openrac::rac1::Item_Give>([](auto original,std::int32_t id,std::int32_t equip){
        original(id,equip);
        last_event="Item acquired: "+std::to_string(id);
    });
    mod.hook<openrac::rac1::Planet_Unlock>([](auto original,std::int32_t level){
        original(level);
        last_event="Planet unlocked: "+g->level_name(level);
    });

    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto mask=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_CIRCLE);
        if(qol::combo_pressed(now,previous,mask)) open_requested=true;
        if((now&mask)==mask) qol::consume_buttons(report,mask);
        previous=now;
    });
    mod.on_frame([]{
        if(open_requested){ open_requested=false; page.open(); }
    });
}
OPENRAC_MOD_CPP("better-hud","0.2.0",load)
