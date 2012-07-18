#ifndef _TECSENTITY_H
#define _TECSENTITY_H

/******************************************************************
*
*  TEscapeEntity.h - Defines the events that can occur
*
*******************************************************************/
#include <stdint.h>
#include "TEscapeTypes.h"
#include <boost/dynamic_bitset.hpp>

namespace Esc
{

class TWorld;
class TSystemManager;
class TGroupManager;

class TEntity
{
public:
    // Use TWorld::CreateEntity instead of accessing directly
    TEntity();
    ~TEntity();

    TComponentPtr GetComponent(std::string componentName);
    TComponentPtr GetComponent(uint64_t typeBit);

    TComponentPtrs &GetComponents() { return Components; }

    uint64_t GetId() { return EntityId; }

    std::string GetGroup() { return GroupName; }

    friend class TWorld;
    friend class TSystemManager;
    friend class TGroupManager;

private:

    void AddComponent(TComponentPtr Component);
    void RemoveComponent(TComponentPtr Component);
    const boost::dynamic_bitset<> &GetComponentBits() { return ComponentBits; }

    bool IsActive;
    std::string GroupName;

    uint64_t EntityId;
    TWorld *World;

    TComponentPtrs Components;
    boost::dynamic_bitset<> ComponentBits;
};

} // namespace

#endif // _TECSENTITY_H
