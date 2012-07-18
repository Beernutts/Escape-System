#ifndef _TECS_H
#define _TECS_H

/******************************************************************
*
*  TEscape.h - Defines the gloabls pieces Ecs Needs
*
*******************************************************************/
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <stdint.h>

#include "TEscapeTypes.h"
namespace Esc
{

class TEntityManager;
class TSystemManager;
class TEntity;
class TComponent;
class TGroupManager;

class TWorld
{
public:
    TWorld();
    ~TWorld();

    static TWorld *GetInstance();

    // These functions should be used to create entities
    TEntityPtr CreateEntity();
    void DeleteEntity(TEntityPtr entity);

    // Adds and remove entity from the world
    void AddEntity(TEntityPtr entity);
    void RemoveEntity(TEntityPtr entity);

    TEntityPtr GetEntity(uint64_t entityId);

    void AddComponent(TEntityPtr entity, TComponentPtr component);
    void RemoveComponent(TEntityPtr entity, TComponentPtr component, bool freeComp = true);

    void AddSystem(TSystemPtr system,
                   uint32_t afterTime = 0, bool isRepeat = false);
    void RemoveSystem(TSystemPtr system);
    void UpdateSystem(TSystemPtr system,
                      uint32_t afterTime, bool isRepeat);

    bool SetGroup(TEntityPtr entity, std::string groupName, bool isAdd = true);
    bool GetGroupEntities(std::string groupName, TEntityPtrs &entities);

    void Update();

    uint32_t GetMilliSecElapsed();

    void SetLocalId(uint32_t localId);
    uint32_t GetLocalId() { return LocalId; }

    uint64_t GetTypeBit(std::string componentType);

private:

    uint32_t LocalId;
    TEntityManager *EntityManager;
    TSystemManager *SystemManager;
    TGroupManager *GroupManager;

    boost::posix_time::ptime StartTime;
    boost::posix_time::ptime CurrentTime;

    struct TSystemInfo
    {
        TSystemPtr system;
        uint32_t afterTime;
        bool isRepeat;
    };

    struct TComponentRemoval
    {
        bool FreeComp;
        TComponentPtr Component;
    };

    struct TComponentAddition
    {
        TEntityPtr Entity;
        TComponentPtr Component;
    };

    std::vector<TSystemInfo> SystemAdditions;
    std::vector<TSystemInfo> SystemUpdates;
    std::vector<TSystemPtr> SystemRemovals;

    std::vector<TComponentAddition> ComponentAdditions;
    std::vector<TComponentRemoval> ComponentRemovals;
    TEntityPtrs EntityDeletetions;
    TEntityPtrs EntityAdditions;
    TEntityPtrs EntityRemovals;

    enum TWorldUpdate {
        COMPONENT_ADDITION,
        COMPONENT_REMOVAL,
        ENTITY_ADDITION,
        ENTITY_REMOVAL,
        ENTITY_DELETION,
        SYSTEM_ADDITION,
        SYSTEM_UPDATE,
        SYSTEM_REMOVAL
    };

    std::vector<TWorldUpdate> WorldUpdateOrder;
};

}

#endif // _TECS_H
