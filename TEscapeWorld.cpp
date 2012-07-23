
/******************************************************************
*
*  TWorld.cpp - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/
#include "TEscapeWorld.h"
#include "TEscapePrivate.h"
#include "TEscapeEntityManager.h"
#include "TEscapeSystemManager.h"
#include "TEscapeGroupManager.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"
#include "TEscapeSystem.h"
#include <stdio.h>

static uint64_t u64UniqueEntityId = 1;
static uint32_t u32LocalId = 0;

namespace Esc {

static TWorld *WorldInstance = NULL;

TWorld::TWorld() :
  LocalId(0)
{
    assert(WorldInstance == NULL);

    EntityManager = new TEntityManager();
    SystemManager = new TSystemManager(this);
    GroupManager = new TGroupManager();

    StartTime = boost::posix_time::microsec_clock::local_time();

    WorldInstance = this;
}

TWorld *TWorld::GetInstance()
{
    if (WorldInstance == NULL)
    {
        WorldInstance = new TWorld();
    }
    return WorldInstance;
}

TWorld::~TWorld()
{
    delete EntityManager;
    delete SystemManager;
}

void TWorld::SetLocalId(uint32_t localId)
{
    LocalId = localId;
    u32LocalId = localId;
}

uint64_t TWorld::GetTypeBit(std::string componentType)
{
    return SystemManager->GetTypeBit(componentType);
}

void TWorld::Reset()
{
    EntityManager->Reset();
    SystemManager->Reset();
}

/********************************************************************
// System functions
********************************************************************/
void TWorld::AddSystem(TSystemPtr system,
                       uint32_t afterTime, bool isRepeat)
{
    TSystemInfo systemAddition;
    systemAddition.system = system;
    systemAddition.afterTime = afterTime;
    systemAddition.isRepeat = isRepeat;

    SystemAdditions.push_back(systemAddition);
    WorldUpdateOrder.push_back(SYSTEM_ADDITION);
}

void TWorld::RemoveSystem(TSystemPtr system, bool freeSystem)
{
    TSystemRemoval sysRemoval;
    sysRemoval.FreeSystem = freeSystem;
    sysRemoval.System = system;
    SystemRemovals.push_back(sysRemoval);
    WorldUpdateOrder.push_back(SYSTEM_REMOVAL);
}

void TWorld::UpdateSystem(TSystemPtr system,
                       uint32_t afterTime, bool isRepeat)
{
    TSystemInfo systemAddition;
    systemAddition.system = system;
    systemAddition.afterTime = afterTime;
    systemAddition.isRepeat = isRepeat;

    SystemUpdates.push_back(systemAddition);
    WorldUpdateOrder.push_back(SYSTEM_UPDATE);
}

/********************************************************************
// Component functions
********************************************************************/
void TWorld::AddComponent(TEntityPtr entity, TComponentPtr component)
{
    // if this entity is in the system already, then we need
    // to make sure the systems update it
    if (entity->IsActive) {
        TComponentAddition compAdd;
        compAdd.Entity = entity;
        compAdd.Component = component;
        ComponentAdditions.push_back(compAdd);
        WorldUpdateOrder.push_back(COMPONENT_ADDITION);
    }
    else {
        entity->AddComponent(component);
    }
}

void TWorld::RemoveComponent(TEntityPtr entity, TComponentPtr component, bool freeComp)
{
    if (entity->IsActive) {
        TComponentRemoval compRemove;
        compRemove.FreeComp = freeComp;
        compRemove.Component = component;
        ComponentRemovals.push_back(compRemove);
        WorldUpdateOrder.push_back(COMPONENT_REMOVAL);
    }
    else {
        entity->RemoveComponent(component);
        if (freeComp) {
            delete component;
        }
    }

}

/********************************************************************
// Entity functions
********************************************************************/
TEntityPtr TWorld::CreateEntity()
{
    TEntityPtr entity;

    // in the future, we can renew entries from EntityManager

    entity = new TEntity();

    return entity;
}

void TWorld::DeleteEntity(TEntityPtr entity)
{
    EntityDeletetions.push_back(entity);
    WorldUpdateOrder.push_back(ENTITY_DELETION);
}

void TWorld::AddEntity(TEntityPtr Entity)
{
    if (!Entity->IsActive) {
        EntityAdditions.push_back(Entity);
        WorldUpdateOrder.push_back(ENTITY_ADDITION);
    }
}

void TWorld::RemoveEntity(TEntityPtr entity)
{
    if (entity->IsActive) {
        EntityRemovals.push_back(entity);
        WorldUpdateOrder.push_back(ENTITY_REMOVAL);
    }
}

TEntityPtr TWorld::GetEntity(uint64_t entityId)
{
    return EntityManager->GetEntity(entityId);
}

/********************************************************************
// Group
********************************************************************/
bool TWorld::SetGroup(TEntityPtr entity, std::string groupName,
                      bool isAdd)
{
    TGroupSet groupInfo;
    groupInfo.Entity = entity;
    groupInfo.GroupName = groupName;
    groupInfo.IsAdd = isAdd;

    if (!isAdd) {
        TEntityPtrs temp;
        if (!GroupManager->Get(groupName, temp)) {
            return false;
        }
    }
    GroupSets.push_back(groupInfo);
    WorldUpdateOrder.push_back(GROUP_SET);

    return true;
}

bool TWorld::GetGroupEntities(std::string groupName, TEntityPtrs &entities)
{
    return GroupManager->Get(groupName, entities);
}

/********************************************************************
// Update
********************************************************************/
void TWorld::Update()
{
    boost::posix_time::ptime lastTime = CurrentTime;
    CurrentTime = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration tickDelta = lastTime - CurrentTime;

    // Update in the order the commands were given
    for (uint32_t i = 0; i < WorldUpdateOrder.size(); i++) {
        switch(WorldUpdateOrder[i]) {
        case COMPONENT_ADDITION:
            if (!ComponentAdditions.empty()) {
                TEntityPtr entity = ComponentAdditions[0].Entity;
                entity->AddComponent(ComponentAdditions[0].Component);
                SystemManager->ComponentAddition(entity, ComponentAdditions[0].Component);
                ComponentAdditions.erase(ComponentAdditions.begin());
            }
            break;

        case COMPONENT_REMOVAL:
            if (!ComponentRemovals.empty()) {
                TEntityPtr entity = ComponentRemovals[0].Component->GetOwnerEntity();
                SystemManager->ComponentRemoval(entity, ComponentRemovals[0].Component);
                entity->RemoveComponent(ComponentRemovals[0].Component);
                if (ComponentRemovals[0].FreeComp) {
                    delete ComponentRemovals[0].Component;
                }
                ComponentRemovals.erase(ComponentRemovals.begin());
            }
            break;

        case ENTITY_ADDITION:
            if (!EntityAdditions.empty()) {
                EntityAdditions[0]->IsActive = true;
                EntityManager->Add(EntityAdditions[0]);
                SystemManager->Add(EntityAdditions[0]);
                EntityAdditions.erase(EntityAdditions.begin());
            }
            break;

        case ENTITY_REMOVAL:
            if (!EntityRemovals.empty()) {
                if (!EntityRemovals[0]->GroupName.empty()) {
                    GroupManager->Set(EntityRemovals[0], EntityRemovals[0]->GroupName, false);
                }
                EntityRemovals[0]->IsActive = false;
                SystemManager->Remove(EntityRemovals[0]);
                EntityManager->Remove(EntityRemovals[0]);
                EntityRemovals.erase(EntityRemovals.begin());
            }
            break;

        case ENTITY_DELETION:
            if (!EntityDeletetions.empty()) {
                if (EntityDeletetions[0]->IsActive) {
                    SystemManager->Remove(EntityDeletetions[0]);
                    EntityManager->Remove(EntityDeletetions[0]);
                }
                delete EntityDeletetions[0];
                EntityDeletetions.erase(EntityDeletetions.begin());
            }
            break;

        case SYSTEM_ADDITION:
            if (!SystemAdditions.empty()) {
                TEntityPtrs entities;
                EntityManager->GetEntities(entities);
                SystemManager->Add(SystemAdditions[0].system,
                               SystemAdditions[0].afterTime,
                               SystemAdditions[0].isRepeat, entities);
                SystemAdditions.erase(SystemAdditions.begin());
            }
            break;

        case SYSTEM_UPDATE:
            if (!SystemUpdates.empty()) {
                TEntityPtrs entities;
                EntityManager->GetEntities(entities);

                SystemManager->UpdateSystem(SystemUpdates[0].system,
                                            SystemUpdates[0].afterTime,
                                            SystemUpdates[0].isRepeat, entities);
                SystemUpdates.erase(SystemUpdates.begin());
            }
            break;

        case SYSTEM_REMOVAL:
            if (!SystemRemovals.empty()) {
                SystemManager->Remove(SystemRemovals[0].System);
                if (SystemRemovals[0].FreeSystem) {
                    delete SystemRemovals[0].System;
                }
                SystemRemovals.erase(SystemRemovals.begin());
            }
            break;

        case GROUP_SET:
            if (!GroupSets.empty()) {
                GroupManager->Set(GroupSets[0].Entity, GroupSets[0].GroupName,
                                  GroupSets[0].IsAdd);
                GroupSets.erase(GroupSets.begin());
            }
            break;
        }
    }
    WorldUpdateOrder.clear();

    // this will loop through all the systems
    SystemManager->Update(static_cast<uint32_t>(tickDelta.total_milliseconds()));
}

uint32_t TWorld::GetMilliSecElapsed()
{
    boost::posix_time::time_duration elapsedTime = CurrentTime - StartTime;

    return static_cast<uint32_t>(elapsedTime.total_milliseconds());
}


/********************************************************************
// Local functions
********************************************************************/
uint64_t GetUniqueEntityId()
{
    uint64_t uniqueId = u64UniqueEntityId | ((uint64_t)u32LocalId << 32);
    u64UniqueEntityId++;

    return uniqueId;
}

} // namespace
