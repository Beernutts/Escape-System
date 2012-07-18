#ifndef _TECSCOMPONENT_H
#define _TECSCOMPONENT_H

/******************************************************************
*
*  TEscapeComponent.h - Defines the events that can occur
*
*******************************************************************/
#include <stdint.h>
#include <string>
#include "TEscapeTypes.h"
#include <typeinfo>

namespace Esc
{

class TComponent
{
public:

    // Use TWorld::DeleteComponent() to delete a component
    TComponent(std::string type);
    virtual ~TComponent();

    std::string GetType() { return Type; }
    uint64_t GetTypeBit() { return TypeBit; }

    void SetEntity(uint64_t entityOwner) { EntityOwner = entityOwner; }
    uint64_t GetEntityId() { return EntityOwner; }

    TEntityPtr GetOwnerEntity();

protected:
    TWorld *World;
    std::string Type;
    uint64_t TypeBit;

    uint64_t EntityOwner;

};


} // namespace

#endif // _TECSCOMPONENT_H
