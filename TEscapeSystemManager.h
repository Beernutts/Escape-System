#ifndef _TECSSYSTEMMANAGER_H
#define _TECSSYSTEMMANAGER_H

/******************************************************************
*
*  TEscapeSystemManager.h -
*
*******************************************************************/
#include <stdint.h>
#include <boost/dynamic_bitset.hpp>
#include "TEscapeTypes.h"
#include <list>

namespace Esc
{

class TWorld;

class TSystemManager
{
public:

    TSystemManager(TWorld *world);
    ~TSystemManager();

    uint64_t GetTypeBit(std::string componentType);

    void Add(TSystemPtr system, uint32_t afterTime, bool isRepeat,
             const TEntityPtrs& entities);
    void Remove(TSystemPtr system);
    void UpdateSystem(TSystemPtr system, uint32_t afterTime, bool isRepeat,
             const TEntityPtrs& entities);

    void Add(TEntityPtr Entity);
    void Remove(TEntityPtr Entity);

    void ComponentAddition(TEntityPtr entity, TComponentPtr component);
    void ComponentRemoval(TEntityPtr Entity, TComponentPtr component);

    void Update(uint32_t tickDelta);

    void Reset();

protected:
    struct TTimedSystem
    {
        uint32_t AtTime;
        uint32_t AfterTime;
        bool IsRepeat;
        TSystemPtr System;
    };

    typedef std::list<TTimedSystem> TTimedSystems;
    typedef std::map<std::string, uint64_t> TBitMap;
    TTimedSystems TimedSystems;
    TSystemPtrs Systems;


    TBitMap BitMap;
    TWorld *World;

};

} // namespace

#endif // _TECSSYSTEMMANAGER_H
