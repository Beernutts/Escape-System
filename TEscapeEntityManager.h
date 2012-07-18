#ifndef _TECSENTITYMANAGER_H
#define _TECSENTITYMANAGER_H

/******************************************************************
*
*  TEntityManager.h - Handles all the entities in the world
*
*******************************************************************/

#include "TEscapeTypes.h"
#include <vector>

namespace Esc {

// temp
class TEntityManager
{
public:
    TEntityManager();
    ~TEntityManager();
    void Add(TEntityPtr Entity);
    void Remove(TEntityPtr Entity);

    void GetEntities(TEntityPtrs& entities);
    TEntityPtr GetEntity(uint64_t entityId);

private:
    TEntityPtrs Entities;

};

} // namespace

#endif // _TECSENTITYMANAGER_H
