#ifndef _TECSGROUPMANAGER_H
#define _TECSGROUPMANAGER_H

/******************************************************************
*
*  TGroupManager.h - Handles all the entities in the world
*
*******************************************************************/

#include "TEscapeTypes.h"
#include <map>

namespace Esc {

// temp
class TGroupManager
{
public:
    TGroupManager();
    ~TGroupManager();

    bool Set(TEntityPtr entity, std::string groupName, bool isAdd);
    bool Get(std::string groupName, TEntityPtrs &entities);

private:
    typedef std::map<std::string,TEntityPtrs> TGroupMap;
    TGroupMap GroupMap;

};

} // namespace

#endif // _TECSGROUPMANAGER_H
