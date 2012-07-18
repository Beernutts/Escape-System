
/******************************************************************
*
*  TComponent.cpp - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/

#include "TEscapeWorld.h"
#include "TEscapeEntityManager.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"
#include "TEscapeComponent.h"

#include <stdio.h>

namespace Esc {

TComponent::TComponent(std::string type) :
  Type(type)
{
    World = TWorld::GetInstance();

    // Get Our Type Bit
    TypeBit = World->GetTypeBit(Type);

}

TComponent::~TComponent()
{
    // ensure this component has been removed from the entity before deletion
    if (EntityOwner != 0) {
        printf("Warning! Component still has Entity Owner!\n");
    }
}

TEntityPtr TComponent::GetOwnerEntity()
{
    return World->GetEntity(EntityOwner);
}

} // namespace
