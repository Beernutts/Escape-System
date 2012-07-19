/******************************************************************
*
*  TEscapeSystem.cpp -
*
*******************************************************************/
#include "TEscapeWorld.h"
#include "TEscapeSystem.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"

namespace Esc
{

TSystem::TSystem(bool alwaysUpdate) :
ComponentBits(64)
{
    World = TWorld::GetInstance();
    AlwaysUpdate = alwaysUpdate;
}

TSystem::~TSystem()
{

}

void TSystem::HandleComponent(std::string componentType, bool isAdd)
{
  ComponentBits.set(World->GetTypeBit(componentType), isAdd);
}

void TSystem::Add(TComponentPtr Component)
{
    Components.push_back(Component);
}

void TSystem::Remove(TComponentPtr Component)
{
    TComponentPtrs::iterator it = Components.begin();

    for (; it != Components.end(); it++) {
        if (*it == Component) {
            Components.erase(it);
            break;
        }
    }
}

void TSystem::Add(TEntityPtr Entity)
{
    Entities.push_back(Entity);
}

void TSystem::Remove(TEntityPtr Entity)
{
    TEntityPtrs::iterator it = Entities.begin();

    for (; it != Entities.end(); it++) {
        if (*it == Entity) {
            Entities.erase(it);
            break;
        }
    }
}

void TSystem::UpdateInternal(uint32_t tickDelta)
{
    if (AlwaysUpdate) {
        Update(Entities, tickDelta);
    }
    else if (!Components.empty()) {
        for (uint32_t i = 0; i < Components.size(); i++) {
            Update(Components[i], tickDelta);
        }
    }
    else {
        for (uint32_t i = 0; i < Entities.size(); i++) {
            Update(Entities[i], tickDelta);
        }
    }
}

} // namespace

