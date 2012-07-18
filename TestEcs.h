#ifndef _SMASHPC_H
#define _SMASHPC_H
/******************************************************************
*
*  SmashPC.h
*
*******************************************************************/
#include "Types.h"
#include <chipmunk.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "TEscapeTypes.h"
#include "TEscapeWorld.h"
#include "TEscapeEntity.h"
#include "TEscapeSystem.h"
#include "TEscapeComponent.h"

#define PI          (3.14159265)


/******************************************************************
*  Components
*******************************************************************/
class TPhysicalObject : public Esc::TComponent
{
public:
    TPhysicalObject(cpSpace *pSpace, cpFloat maxSpeed);
    TPhysicalObject(cpSpace *pSpace, cpBody *pBody, cpShape *pShape, cpFloat maxSpeed);
    ~TPhysicalObject();

    void SetBody(cpBody *pBody);
    cpBody *GetBody();

    void SetShape(cpShape *pShape);
    cpShape *GetShape();

    cpFloat GetMaxSpeed() { return MaxSpeed; }

private:
    cpSpace *mpSpace;
    cpBody *mpBody;
    cpShape *mpShape;

    cpFloat MaxSpeed;

};

/******************************************************************/
class THealth : public Esc::TComponent
{
public:
    THealth(int maxHealth, int StartingHealth) :
      Esc::TComponent("Health"), MaxHealth(maxHealth), CurrentHealth(StartingHealth){}

    void AddDamage(int Damage) { CurrentHealth -= Damage; if (CurrentHealth < 0) CurrentHealth = 0; }
    void AddHealth(int Damage) {CurrentHealth += Damage; if (CurrentHealth > MaxHealth) CurrentHealth = MaxHealth; };

    int GetHealth() { return CurrentHealth; }
    int GetHealthPercetage() { return ((CurrentHealth * 100) / MaxHealth); }

    bool IsDead() { return CurrentHealth == 0; }

private:
    int MaxHealth;
    int CurrentHealth;

};

/******************************************************************/
class TDamage : public Esc::TComponent
{
public:
    TDamage(int damagevalue) : Esc::TComponent("Damage"), DamageValue(damagevalue) {}

    int GetDamage() { return DamageValue; }

private:
    int DamageValue;
};

/******************************************************************/
class TEnemy : public Esc::TComponent
{
public:
    TEnemy() : Esc::TComponent("Enemy") {}

};

/******************************************************************/
class TInput : public Esc::TComponent
{
public:
    TInput() : Esc::TComponent("Input") {}

};

/******************************************************************/
class TCollision : public Esc::TComponent
{
public:
    TCollision(Esc::TEntityPtr entity) : Esc::TComponent("Collision") { NewEntities.push_back(entity); }

    void SetEntityCollide(Esc::TEntityPtr entity, bool isCollde = true)
    {
        if (isCollde) {
            NewEntities.push_back(entity);
        }
        else {
            uint32_t i;
            for (i = 0; i < Entities.size(); i++) {
                if (Entities[i] == entity) {
                    Entities.erase(Entities.begin()+i);
                    break;
                }
            }
            if (i == Entities.size()) {
                for (i = 0; i < NewEntities.size(); i++) {
                    if (NewEntities[i] == entity) {
                        NewEntities.erase(NewEntities.begin()+i);
                        break;
                    }
                }
            }
        }
    }
    void GetNewCollisionEntity(Esc::TEntityPtr &entity)
    {
        entity = NewEntities.front();
        // Copy New Entity to full list
        Entities.push_back(NewEntities.front());
        NewEntities.erase(NewEntities.begin());
    }
    void GetEntitiesColliding(Esc::TEntityPtrs &entities) { entities = Entities; }
    int GetCollidingCount() { return Entities.size(); }
    bool IsNewCollision() { return !NewEntities.empty(); }

private:
    Esc::TEntityPtrs Entities;
    Esc::TEntityPtrs NewEntities;

};

/******************************************************************/
class TGraphicsObject : public Esc::TComponent
{
public:
    TGraphicsObject(sf::Image *image) : Esc::TComponent("GraphicsObject"), SfmlImage(image)
    {
        if (image) {
            SfmlSprite = new sf::Sprite();
            SfmlSprite->SetImage(*SfmlImage);
            SfmlSprite->SetOrigin(SfmlImage->GetWidth()/2, SfmlImage->GetHeight()/2);

            SfmlSprite->SetRotation(0*(180.0f/PI));
        }
    }
    ~TGraphicsObject() { delete SfmlSprite; }

    bool IsImage() { return (SfmlImage != NULL); }
    sf::Sprite *GetSprite() { return SfmlSprite; }

    void GetSize(uint32_t &width, uint32_t &height)
    {
        width = SfmlImage->GetWidth();
        height = SfmlImage->GetHeight();
    }

private:
    sf::Image *SfmlImage;
    sf::Sprite *SfmlSprite;

};

/******************************************************************
*  Systems
*******************************************************************/
class TPhysicsSystem : public Esc::TSystem
{
public:
    TPhysicsSystem(cpSpace *pSpace);
    ~TPhysicsSystem();

    void Update(Esc::TComponentPtr component, uint32_t tickDelta);
    void PreStep();
    void Initialize();

private:
    cpSpace *mpSpace;
    cpVect LastPos;
    uint32_t LastUpdate;

};

/******************************************************************/
class TRenderingSystem : public Esc::TSystem
{
public:
    TRenderingSystem(sf::RenderWindow *pApp);
    ~TRenderingSystem();

    void Update(Esc::TEntityPtr entity, uint32_t tickDelta);
    void Initialize();

private:
    sf::RenderWindow *mpApp;

};

/******************************************************************/
class TInputSystem : public Esc::TSystem
{
public:
    TInputSystem(sf::RenderWindow *pApp);
    ~TInputSystem();

    void Update(Esc::TEntityPtr entity, uint32_t tickDelta);
    void Initialize();

private:
    sf::RenderWindow *mpApp;

    uint32_t mCenterY;
    uint32_t mCenterX;

};

/******************************************************************/
class TDamageSystem : public Esc::TSystem
{
public:
    TDamageSystem();
    ~TDamageSystem();

    void Update(Esc::TEntityPtr entity, uint32_t tickDelta);
    void Initialize();
};

/******************************************************************/
class TEnemySystem : public Esc::TSystem
{
public:
    TEnemySystem();
    ~TEnemySystem();

    void Update(Esc::TComponentPtr component, uint32_t tickDelta);
    void Initialize();
private:
    Esc::TWorld *World;
};

/******************************************************************/
class TDeathSystem : public Esc::TSystem
{
public:
    TDeathSystem(cpSpace *pSpace);
    ~TDeathSystem();

    void Update(Esc::TComponentPtr component, uint32_t tickDelta);
    void Initialize();
private:
    Esc::TWorld *World;
    cpSpace *mpSpace;
};

/******************************************************************
*  Game
*******************************************************************/
class Game
{
public:
    Game(sf::RenderWindow *app);
    ~Game();

    void Run();

private:

    bool CheckGameEnd();



    sf::RenderWindow *pApp;
    Esc::TWorld *pWorld;
    cpSpace *mpSpace;

    TPhysicsSystem *PhysicsSystem;
    TRenderingSystem *RenderingSystem;
    TDamageSystem *DamageSystem;
    TDeathSystem *DeathSystem;
    TInputSystem *InputSystem;
    TEnemySystem *EnemySystem;

};

int OnCollision(cpArbiter *arb,
                cpSpace *space, void *data);
void OnSeparate(cpArbiter *arb,
                cpSpace *space, void *data);
void SetupCollision(cpSpace *pSpace,
                    Esc::TEntityPtr Entity1, cpShape *pShape1,
                    Esc::TEntityPtr Entity2, cpShape *pShape2, bool enable = true);

#endif // _SMASHPC_H
