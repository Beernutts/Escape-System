
/******************************************************************
*
*  TEntityManager.cpp - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/

#include "TEscapeWorld.h"
#include "TEscapeEntityManager.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"

namespace Esc {


TEntityManager::TEntityManager()
{

}

TEntityManager::~TEntityManager()
{

}

void TEntityManager::Add(TEntityPtr Entity)
{
    Entities.push_back(Entity);

}

void TEntityManager::Remove(TEntityPtr Entity)
{
    for (uint32_t i = 0; i < Entities.size(); i++) {
        if (Entities[i] == Entity) {
            Entities.erase(Entities.begin() + i);
            break;
        }
    }
}

TEntityPtr TEntityManager::GetEntity(uint64_t entityId)
{

    for (uint32_t i = 0; i < Entities.size(); i++) {
        if (Entities[i]->GetId() == entityId) {
            return Entities[i];
        }
    }

    return NULL;
}

void TEntityManager::GetEntities(TEntityPtrs& entities)
{
    entities = Entities;
}

void TEntityManager::Reset()
{
    for (uint32_t i = 0; i < Entities.size(); i++) {
        TComponentPtrs components = Entities[i]->GetComponents();
        for (uint32_t j = 0; j < components.size(); j++) {
            Entities[i]->RemoveComponent(components[j]);
            delete components[j];
        }
        delete Entities[i];
    }
}

} // namespace
