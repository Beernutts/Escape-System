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
    virtual void Update(bool AlwaysUpdate, uint32_t tickDelta) {}
    virtual void PreStep() {};

    void HandleComponent(std::string componentType, bool isAdd = true);

    friend class TSystemManager;

protected:
    TWorld *World;

private:
    void Update(uint32_t tickDelta);

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

} // namespace

#endif // _TECSSYSTEM_H
