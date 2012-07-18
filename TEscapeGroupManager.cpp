
/******************************************************************
*
*  TGroupManager.cpp - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/

#include "TEscapeWorld.h"
#include "TEscapeGroupManager.h"
#include "TEscapePrivate.h"
#include "TEscapeEntity.h"

namespace Esc {


TGroupManager::TGroupManager()
{

}

TGroupManager::~TGroupManager()
{

}

bool TGroupManager::Set(TEntityPtr entity, std::string groupName, bool isAdd)
{
    bool successFlag = false;
    if (isAdd) {
        GroupMap[groupName].push_back(entity);
        entity->GroupName = groupName;
        successFlag = true;
    }
    else {
        TGroupMap::iterator it = GroupMap.begin();
        for (; it != GroupMap.end(); it++) {
            if (it->first == groupName) {
                uint32_t i;
                for (i = 0; i < it->second.size(); i++) {
                    if (it->second[i] == entity) {
                        it->second.erase(it->second.begin()+i);
                        entity->GroupName.clear();
                        successFlag = true;
                        break;
                    }
                }
            }
        }
    }

    return successFlag;
}

bool TGroupManager::Get(std::string groupName, TEntityPtrs &entities)
{
    bool successFlag = false;
    TGroupMap::iterator it = GroupMap.begin();
    for (; it != GroupMap.end(); it++) {
        if (it->first == groupName) {
            entities = it->second;
            successFlag = true;
            break;
        }
    }

    return successFlag;
}


} // namespace
