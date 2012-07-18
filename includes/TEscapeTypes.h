#ifndef _TECSTYPES_H
#define _TECSTYPES_H

/******************************************************************
*
*  TEscapeTypes.h - Defines the types for Ecs
*
*******************************************************************/
#include <stdint.h>
#include <vector>
#include <string>

namespace Esc
{

class TEntity;
class TComponent;
class TSystem;

typedef TEntity* TEntityPtr;
typedef TComponent* TComponentPtr;
typedef TSystem* TSystemPtr;

typedef std::vector<TEntityPtr> TEntityPtrs;
typedef std::vector<TComponentPtr> TComponentPtrs;
typedef std::vector<TSystemPtr> TSystemPtrs;


}

#endif // _TECSTYPES_H
