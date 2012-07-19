#ifndef _TECSSYSTEM_H
#define _TECSSYSTEM_H

/******************************************************************
*
*  TEscapeSystem.h -
*
*******************************************************************/
#include <stdint.h>
#include <boost/dynamic_bitset.hpp>
#include "TEscapeTypes.h"

namespace Esc
{

class TWorld;
class TSystemManager;

class TSystem
{
public:

    TSystem(bool alwaysUpdate = false);
    ~TSystem();

    virtual void Initialize() = 0;
    virtual void Update(TComponentPtr component, uint32_t tickDelta) {}
    virtual void Update(TEntityPtr entity, uint32_t tickDelta) {}

    // Called once for systems that always want update run 1 per tick
    virtual void Update(TEntityPtrs entities, uint32_t tickDelta) {}
    virtual void PreStep() {};
    virtual void PostStep() {};

    void HandleComponent(std::string componentType, bool isAdd = true);

    friend class TSystemManager;

protected:
    TWorld *World;

private:
    void UpdateInternal(uint32_t tickDelta);

    bool AllEntities() { return AlwaysUpdate; }

    // Internal Functions
    const boost::dynamic_bitset<> &GetComponentBits() { return ComponentBits; }

    void Add(TComponentPtr Component);
    void Remove(TComponentPtr Component);

    void Add(TEntityPtr Entity);
    void Remove(TEntityPtr Entity);

    boost::dynamic_bitset<> ComponentBits;
    TComponentPtrs Components;
    TEntityPtrs Entities;
    bool AlwaysUpdate;

};

class TComponentSystem: public TSystem
{

};

} // namespace

#endif // _TECSSYSTEM_H
