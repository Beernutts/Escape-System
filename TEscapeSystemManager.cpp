/******************************************************************
*
*  TEscapeSystemManager.cpp -
*
*******************************************************************/
#include "TEscapeWorld.h"
#include "TEscapeSystemManager.h"
#include "TEscapeSystem.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"

#include <stdio.h>

static uint64_t u32TypeBit = 0;

namespace Esc
{

TSystemManager::TSystemManager(TWorld *world)
{
    World = world;
}

TSystemManager::~TSystemManager()
{

}

uint64_t TSystemManager::GetTypeBit(std::string componentType)
{
    TBitMap::iterator it = BitMap.begin();

    for (; it != BitMap.end(); it++) {
        if (it->first == componentType) {
            return it->second;
        }
    }

    // Add it in
    BitMap[componentType] = u32TypeBit++;

    return BitMap[componentType];
}

void TSystemManager::Add(TSystemPtr system, uint32_t afterTime, bool isRepeat,
                         const TEntityPtrs& entities)
{
    system->Initialize();

    if (afterTime) {
        TTimedSystem sysInfo;
        sysInfo.AtTime = afterTime + World->GetMilliSecElapsed();
        sysInfo.AfterTime = afterTime;
        sysInfo.IsRepeat = isRepeat;
        sysInfo.System = system;

        // insert in order
        TTimedSystems::iterator it = TimedSystems.begin();
        for (; it != TimedSystems.end(); it++) {
            if (it->AtTime > sysInfo.AtTime) {
                TimedSystems.insert(it, sysInfo);
                return;
            }
        }

        TimedSystems.insert(it, sysInfo);


    }
    else {
        Systems.push_back(system);
    }

    // Update the entities/components for this system
    const boost::dynamic_bitset<> &SystemBits = system->GetComponentBits();
    for (uint32_t i = 0; i < entities.size(); i++) {
        const boost::dynamic_bitset<> &EntityBits = entities[i]->GetComponentBits();
        if (SystemBits.is_subset_of(EntityBits)) {
            if (SystemBits.count() == 1) {
                system->Add(entities[i]->GetComponent(SystemBits.find_first()));
            }
            else {
                system->Add(entities[i]);
            }
        }
    }
}

void TSystemManager::UpdateSystem(
    TSystemPtr system, uint32_t afterTime, bool isRepeat,
    const TEntityPtrs& entities)
{
    TTimedSystems::iterator it = TimedSystems.begin();
    bool isFound = false;

    // Find the system
    for (uint32_t i = 0; i < Systems.size() + TimedSystems.size(); i++) {
        if (i < Systems.size()) {
            if (system == Systems[i]) {
                if (afterTime) {
                    TTimedSystem sysInfo;
                    sysInfo.AtTime = afterTime + World->GetMilliSecElapsed();
                    sysInfo.AfterTime = afterTime;
                    sysInfo.IsRepeat = isRepeat;
                    sysInfo.System = system;
                    // Move from Systems list to timed list
                    TTimedSystems::iterator it2 = TimedSystems.begin();
                    for (; it2 != TimedSystems.end(); it2++) {
                        if (it2->AtTime > sysInfo.AtTime) {
                            TimedSystems.insert(it2, sysInfo);
                            break;
                        }
                    }
                    if (it2 == TimedSystems.end()) {
                        TimedSystems.insert(it2, sysInfo);
                    }
                }
                isFound = true;
                break;
            }
        }
        else {
            if (system == it->System) {
                if (afterTime == 0) {
                    // move to regular system list
                    TimedSystems.erase(it);
                    Systems.push_back(system);
                }
                else if (afterTime != it->AfterTime) {
                    it->AfterTime = afterTime;
                    it->AtTime = afterTime + World->GetMilliSecElapsed();
                }
                else {
                    it->IsRepeat = isRepeat;
                }
                isFound = true;
                break;
            }
            it++;
        }
    }

    if (!isFound) {
        Add(system, afterTime, isRepeat, entities);
    }
}

void TSystemManager::Remove(TSystemPtr system)
{
    // Check System list first
    for (uint32_t i = 0; i < Systems.size(); i++) {
        if (Systems[i] == system) {
            Systems.erase(Systems.begin() + i);
            return;
        }
    }

    TTimedSystems::iterator it = TimedSystems.begin();
    for (; it != TimedSystems.end(); it++) {
        if (it->System == system) {
            TimedSystems.erase(it);
            return;
        }
    }
    printf("Failed to Find System to remove!\n");
}

void TSystemManager::ComponentAddition(TEntityPtr entity, TComponentPtr component)
{
    const boost::dynamic_bitset<> &EntityBits = entity->GetComponentBits();
    TTimedSystems::iterator it = TimedSystems.begin();
    TSystemPtr system;
    boost::dynamic_bitset<> ComponentBit(ECS_MAX_COMPONENTS);
    ComponentBit.set(component->GetTypeBit());

    //printf("In ComponentAddition... ");


    for (uint32_t i = 0; i < Systems.size() + TimedSystems.size(); i++) {
        if (i < Systems.size()) {
            system = Systems[i];
        }
        else {
            system = it->System;
            it++;
        }
        const boost::dynamic_bitset<> &SystemBits = system->GetComponentBits();
        // Check if this system only wants one component
        if (ComponentBit == SystemBits) {
            //printf("Adding Component%s:%d singly\n", component->GetType().c_str(), (uint32_t)component->GetTypeBit());
            system->Add(component);
        }
        // check if adding this component made this entity
        // eligible for a new system
        else if (SystemBits.is_subset_of(EntityBits) &&
                 ComponentBit.is_subset_of(SystemBits)) {
            //printf("Adding Component%s:%d whole Entity\n", component->GetType().c_str(), (uint32_t)component->GetTypeBit());
            system->Add(entity);
        }
    }
}

void TSystemManager::ComponentRemoval(TEntityPtr entity, TComponentPtr component)
{
    const boost::dynamic_bitset<> &EntityBits = entity->GetComponentBits();
    TTimedSystems::iterator it = TimedSystems.begin();
    TSystemPtr system;
    boost::dynamic_bitset<> ComponentBit(ECS_MAX_COMPONENTS);
    ComponentBit.set(component->GetTypeBit());

    //printf("In ComponentRemoval... component %s: %d\n",
    //       component->GetType().c_str(), (uint32_t)component->GetTypeBit());

       for (uint32_t i = 0; i < Systems.size() + TimedSystems.size(); i++) {
        if (i < Systems.size()) {
            system = Systems[i];
        }
        else {
            system = it->System;
            it++;
        }
        const boost::dynamic_bitset<> &SystemBits = system->GetComponentBits();

        //printf("SystemBits %x EntityBits %x CompBits %x\n", SystemBits.to_ulong(),
        //       EntityBits.to_ulong(), ComponentBit.to_ulong());

        // Check if this system only wants one component
        if (ComponentBit == SystemBits) {
            //printf("Removing Component singly\n\n");
            system->Remove(component);
        }
        // check if adding this component made this entity
        // eligible for a new system
        else if (SystemBits.is_subset_of(EntityBits) &&
                 ComponentBit.is_subset_of(SystemBits)) {
            //printf("Removing whole Entity\n\n");
            system->Remove(entity);
        }
    }
}

void TSystemManager::Add(TEntityPtr Entity)
{
    const boost::dynamic_bitset<> &EntityBits = Entity->GetComponentBits();
    TTimedSystems::iterator it = TimedSystems.begin();
    TSystemPtr system;

    // Get the bits from the components, and match them to the bits in the systems
    // if a system has one bit, then assign that component, otherwise
    // assign the entity
    for (uint32_t i = 0; i < Systems.size() + TimedSystems.size(); i++) {
        if (i < Systems.size()) {
            system = Systems[i];
        }
        else {
            system = it->System;
            it++;
        }
        const boost::dynamic_bitset<> &SystemBits = system->GetComponentBits();
        if (SystemBits.is_subset_of(EntityBits)) {
            if (SystemBits.count() == 1) {
                system->Add(Entity->GetComponent(SystemBits.find_first()));
            }
            else {
                system->Add(Entity);
            }
        }
    }
}

void TSystemManager::Remove(TEntityPtr Entity)
{
    const boost::dynamic_bitset<> &EntityBits = Entity->GetComponentBits();
    TTimedSystems::iterator it = TimedSystems.begin();
    TSystemPtr system;

    // Get the bits from the components, and match them to the bits in the systems
    // if a system has one bit, then assign that component, otherwise
    // assign the entity
    for (uint32_t i = 0; i < Systems.size() + TimedSystems.size(); i++) {
        if (i < Systems.size()) {
            system = Systems[i];
        }
        else {
            system = it->System;
            it++;
        }
        const boost::dynamic_bitset<> &SystemBits = system->GetComponentBits();
        if (SystemBits.is_subset_of(EntityBits)) {
            if (SystemBits.count() == 1) {
                system->Remove(Entity->GetComponent(SystemBits.find_first()));
            }
            else {
                system->Remove(Entity);
            }
        }
    }
}

void TSystemManager::Update(uint32_t tickDelta)
{
    // perform pre-step for all systems about to run
    for (uint32_t i = 0; i < Systems.size(); i++) {
        Systems[i]->PreStep();
    }

    for(TTimedSystems::iterator it = TimedSystems.begin();
        it != TimedSystems.end() &&
          it->AtTime <= World->GetMilliSecElapsed(); it++) {
        TTimedSystem sysInfo = *it;
        sysInfo.System->PreStep();
    }

    // perform the actual updates
    for (uint32_t i = 0; i < Systems.size(); i++) {
        Systems[i]->Update(tickDelta);
    }

    while(TimedSystems.size() &&
          TimedSystems.front().AtTime <= World->GetMilliSecElapsed()) {
        TTimedSystem sysInfo = TimedSystems.front();
        TimedSystems.pop_front();
        sysInfo.System->Update(tickDelta);
        if (sysInfo.IsRepeat) {
            sysInfo.AtTime = sysInfo.AfterTime + World->GetMilliSecElapsed();
            // Place it back into the list
            // insert in order
            TTimedSystems::iterator it = TimedSystems.begin();
            for (; it != TimedSystems.end(); it++) {
                if (it->AtTime > sysInfo.AtTime) {
                    TimedSystems.insert(it, sysInfo);
                }
            }
            if (it == TimedSystems.end()) {
                TimedSystems.insert(it, sysInfo);
            }
        }
        else {
            Remove(sysInfo.System);
        }
    }

}


} // namespace
