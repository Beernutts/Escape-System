#ifndef _TECSPRIVATE_H
#define _TECSPRIVATE_H

/******************************************************************
*
*  TEscapePrivate.h - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/
#include <stdint.h>
#include "TEscapeTypes.h"

#define ECS_MAX_COMPONENTS  64

namespace Esc
{

uint64_t GetUniqueEntityId();

}

#endif // _TECSPRIVATE_H
