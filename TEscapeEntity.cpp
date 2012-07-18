
/******************************************************************
*
*  TEntity.cpp - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/

#include "TEscapeWorld.h"
#include "TEscapeEntityManager.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"

#include <stdio.h>

namespace Esc {


TEntity::TEntity() :
  IsActive(false),
  ComponentBits(ECS_MAX_COMPONENTS)
{
    World = TWorld::GetInstance();

    EntityId = GetUniqueEntityId();
}

TEntity::~TEntity()
{
    // delete all the components
    for (uint32_t i = 0; i < Components.size(); i++) {
        Components[i]->SetEntity(0);
        delete Components[i];
    }
    Components.clear();
}

void TEntity::AddComponent(TComponentPtr Component)
{
    Component->SetEntity(EntityId);
    Components.push_back(Component);

    ComponentBits.set(Component->GetTypeBit());

}

void TEntity::RemoveComponent(TComponentPtr Component)
{
    TComponentPtrs::iterator it = Components.begin();

    for (; it != Components.end(); it++) {
        if (*it == Component) {
            break;
        }
    }

    if (it != Components.end()) {
        ComponentBits.set(Component->GetTypeBit(), false);
        Component->SetEntity(0);
        Components.erase(it);
    }
}

TComponentPtr TEntity::GetComponent(std::string componentName)
{
    for (uint32_t i = 0; i < Components.size(); i++) {
        if (Components[i]->GetType() == componentName) {
            return Components[i];
        }
    }

    return NULL;
}

TComponentPtr TEntity::GetComponent(uint64_t typeBit)
{
    for (uint32_t i = 0; i < Components.size(); i++) {
        if (Components[i]->GetTypeBit() == typeBit) {
            return Components[i];
        }
    }

    printf("Did not find Component typebit %u!\n", (unsigned int)typeBit);

    return NULL;
}

} // namespace
