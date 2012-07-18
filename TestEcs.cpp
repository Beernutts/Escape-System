
#include <stdlib.h>
#include <stdio.h>
#include <chipmunk.h>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <math.h>
#include <windows.h>
#include <Mmsystem.h>
#include "TestEcs.h"
#include "Utilities.h"

int main(int argc, char *argv[])
{
    U32  u32ScreenX, u32ScreenY;
    BOOL bFullScreen;

    //GameSound::Init((char *)"SmashPcSounds.xml");
    Utilities::LoadConfig((char *)"GameCfg.xml");

    Utilities::ScreenResolutionGet(u32ScreenX, u32ScreenY, bFullScreen);

	sf::RenderWindow App(sf::VideoMode(u32ScreenX, u32ScreenY, 32), "SmashPC", (bFullScreen ? sf::Style::Fullscreen : sf::Style::Default));

    Game game(&App);

    game.Run();

    App.Close();

}

Game::Game(sf::RenderWindow *app) :
  pApp(app)
{
    pWorld = Esc::TWorld::GetInstance();

    cpInitChipmunk();

    mpSpace = cpSpaceNew();

    PhysicsSystem = new TPhysicsSystem(mpSpace);
    RenderingSystem = new TRenderingSystem(app);
    DamageSystem = new TDamageSystem();
    DeathSystem = new TDeathSystem(mpSpace);
    InputSystem = new TInputSystem(app);
    EnemySystem = new TEnemySystem();

    pWorld->AddSystem(InputSystem);
    pWorld->AddSystem(EnemySystem, 1000, true);
    pWorld->AddSystem(PhysicsSystem);
    pWorld->AddSystem(RenderingSystem);
    pWorld->AddSystem(DamageSystem);
    pWorld->AddSystem(DeathSystem);

    Esc::TEntityPtr Entity1;
    Esc::TEntityPtr Entity2;

    Entity1 = pWorld->CreateEntity();
    TGraphicsObject* graphicComp(new TGraphicsObject(Utilities::ImageGet("gfx/player.bmp")));
    TPhysicalObject* physicalComp;
    THealth* healthComp(new THealth(100, 100));
    TInput* inputComp(new TInput());

    // Create a Physics object
    uint32_t width, height;
    graphicComp->GetSize(width, height);

    cpBody *pBody = cpBodyNew(1,
                           cpMomentForCircle(1, (width+height)/4,
                                             0.0f, cpvzero));
    pBody->p = cpv(pApp->GetWidth()/2,pApp->GetHeight()/2);
    //pBody->v_limit = 200.0f;

    cpShape *pShape = cpCircleShapeNew(pBody, (width+height)/4, cpvzero);

    pShape->collision_type = 1;
	pShape->layers = 1;

	pShape->e = 0.0f;  // not elatics, no bouncing
	pShape->u = 0.0f; // 0.0 is frictionless

    cpSpaceAddBody(mpSpace, pBody);
    cpSpaceAddShape(mpSpace, pShape);

    physicalComp = new TPhysicalObject(mpSpace, pBody, pShape, 200.0f);

    pWorld->AddComponent(Entity1, graphicComp);
    pWorld->AddComponent(Entity1, physicalComp);
    pWorld->AddComponent(Entity1, healthComp);
    pWorld->AddComponent(Entity1, inputComp);

    pWorld->SetGroup(Entity1, "Player");
    pWorld->AddEntity(Entity1);

    // Add Enemy
    Entity2 = pWorld->CreateEntity();
    TGraphicsObject* graphicComp2(new TGraphicsObject(Utilities::ImageGet("gfx/blueenemy.bmp")));
    TPhysicalObject* physicalComp2;
    TDamage* damageComp2(new TDamage(10));
    TEnemy* enemyComp2(new TEnemy());

    // Create a Physics object
    graphicComp->GetSize(width, height);

    cpBody *pBody2 = cpBodyNew(1,
                           cpMomentForCircle(1, (width+height)/4,
                                             0.0f, cpvzero));
    pBody2->p = cpv(400,400);
    //pBody2->v_limit = 150.0f;

    cpShape *pShape2 = cpCircleShapeNew(pBody2, (width+height)/4, cpvzero);
    pShape2->collision_type = 2;
	pShape2->layers = 1;

	pShape2->e = 0.0f;  // not elatics, no bouncing
	pShape2->u = 0.0f; // 0.0 is frictionless

    cpSpaceAddBody(mpSpace, pBody2);
    cpSpaceAddShape(mpSpace, pShape2);

    physicalComp2 = new TPhysicalObject(mpSpace, pBody2, pShape2, 150.0f);

    pWorld->AddComponent(Entity2, graphicComp2);
    pWorld->AddComponent(Entity2, physicalComp2);
    pWorld->AddComponent(Entity2, damageComp2);
    pWorld->AddComponent(Entity2, enemyComp2);

    pWorld->AddEntity(Entity2);

    // Set up the collisions callsbacks
    SetupCollision(mpSpace, Entity1, pShape, Entity2, pShape2);

}

Game::~Game()
{

}

void Game::Run()
{

    pApp->ShowMouseCursor(true);
    pApp->SetCursorPosition(pApp->GetWidth()/2, pApp->GetHeight()/2);

    pApp->EnableVerticalSync(true);
    pApp->SetFramerateLimit(60);

    while(pApp->IsOpened()) {

        /* clear screen and draw map */
        pApp->Clear(sf::Color(200, 200, 200, 255));

        sf::Shape oShape;

        oShape = sf::Shape::Circle(840.0f, 525.0f, 500.0f, sf::Color(200, 200, 200, 255), 5);
        pApp->Draw(oShape);

        pWorld->Update();

        pApp->Display();

        if (CheckGameEnd())
        {
            pApp->Close();
            break;
        }
    }
}

void SetupCollision(cpSpace *pSpace, Esc::TEntityPtr Entity1, cpShape *pShape1,
                          Esc::TEntityPtr Entity2, cpShape *pShape2, bool enable)
{
    if (enable) {
        pShape1->data = (cpDataPointer)Entity1;
        pShape2->data = (cpDataPointer)Entity2;

        cpSpaceAddCollisionHandler(pSpace, pShape1->collision_type,
                                   pShape2->collision_type,
                                   OnCollision, NULL, NULL, OnSeparate, NULL);
    }
    else {
        cpSpaceRemoveCollisionHandler(pSpace, pShape1->collision_type,
                                      pShape2->collision_type);
    }
}

int OnCollision(cpArbiter *arb,
                      cpSpace *space, void *data)
{
    Esc::TEntity *pEntity1, *pEntity2;
    cpShape *pShape1, *pShape2;
    cpArbiterGetShapes(arb, &pShape1, &pShape2);

    printf("OnCollision!\n");

    pEntity1 = (Esc::TEntity*)pShape1->data;
    pEntity2 = (Esc::TEntity*)pShape2->data;

    // Add Collision component to each of these pointing to the other
    TCollision *collisionComp =
        static_cast<TCollision*>(pEntity1->GetComponent("Collision"));

    // Check if either already has a collision component
    if (collisionComp == NULL) {
        collisionComp = new TCollision(pEntity2);
    }
    else {
        collisionComp->SetEntityCollide(pEntity2);
    }
    Esc::TWorld::GetInstance()->AddComponent(pEntity1, collisionComp);

    // Do same for other entity
    collisionComp =
        static_cast<TCollision*>(pEntity2->GetComponent("Collision"));

    // Check if either already has a collision component
    if (collisionComp == NULL) {
        collisionComp = new TCollision(pEntity1);
    }
    else {
        collisionComp->SetEntityCollide(pEntity1);
    }
    Esc::TWorld::GetInstance()->AddComponent(pEntity2, collisionComp);

    // 0 =  no more processing needed: Do I need it at 1 to get separate?
    return 1;
}

void OnSeparate(cpArbiter *arb,
                      cpSpace *space, void *data)
{
    Esc::TEntity *pEntity1, *pEntity2;
    cpShape *pShape1, *pShape2;
    cpArbiterGetShapes(arb, &pShape1, &pShape2);

    printf("OnSeparate!\n");

    pEntity1 = (Esc::TEntity*)pShape1->data;
    pEntity2 = (Esc::TEntity*)pShape2->data;

    // Remove collisions
    TCollision *collisionComp =
        static_cast<TCollision*>(pEntity1->GetComponent("Collision"));

    if (collisionComp->GetCollidingCount() > 1) {
        collisionComp->SetEntityCollide(pEntity2, false);
    }
    else {
        Esc::TWorld::GetInstance()->RemoveComponent(pEntity1, collisionComp);
    }

    collisionComp =
        static_cast<TCollision*>(pEntity2->GetComponent("Collision"));

    if (collisionComp->GetCollidingCount() > 1) {
        collisionComp->SetEntityCollide(pEntity1, false);
    }
    else {
        Esc::TWorld::GetInstance()->RemoveComponent(pEntity2, collisionComp);
    }

}

/******************************************************************
*  Components
*******************************************************************/
TPhysicalObject::TPhysicalObject(cpSpace *pSpace, cpFloat maxSpeed) :
  Esc::TComponent("PhysicalObject"),
  mpSpace(pSpace),
  mpBody(NULL),
  mpShape(NULL),
  MaxSpeed(maxSpeed)
{

}

TPhysicalObject::TPhysicalObject(cpSpace *pSpace, cpBody *pBody, cpShape *pShape, cpFloat maxSpeed) :
  Esc::TComponent("PhysicalObject"),
  mpSpace(pSpace),
  mpBody(pBody),
  mpShape(pShape),
  MaxSpeed(maxSpeed)
{
    // Setup the User Data
}

TPhysicalObject::~TPhysicalObject()
{
    if (mpBody && mpShape) {
        cpSpaceRemoveBody(mpSpace, mpBody);
        cpSpaceRemoveShape(mpSpace, mpShape);
        cpBodyFree(mpBody);
        cpShapeFree(mpShape);
    }
}

void TPhysicalObject::SetBody(cpBody *pBody)
{
    mpBody = pBody;
}

cpBody *TPhysicalObject::GetBody()
{
    return mpBody;
}

void TPhysicalObject::SetShape(cpShape *pShape)
{
    mpShape = pShape;
}

cpShape *TPhysicalObject::GetShape()
{
    return mpShape;
}

/******************************************************************
*  Systems
*******************************************************************/
TPhysicsSystem::TPhysicsSystem(cpSpace *pSpace) :
  Esc::TSystem(), mpSpace(pSpace), LastUpdate(0)
{
    cpSpaceInit(mpSpace);
    mpSpace->iterations = 10;

}

TPhysicsSystem::~TPhysicsSystem()
{
    cpSpaceFree(mpSpace);
}
void TPhysicsSystem::PreStep()
{
    if (LastUpdate == 0) {
        cpSpaceStep(mpSpace, 1.0f/60.0f);
    }
    else {
        cpSpaceStep(mpSpace, (cpFloat)(World->GetMilliSecElapsed() - LastUpdate)/1000.0f);
    }
    LastUpdate = World->GetMilliSecElapsed();
}

void TPhysicsSystem::Update(Esc::TComponentPtr component, uint32_t tickDelta)
{
    TPhysicalObject *object = static_cast<TPhysicalObject*>(component);

    if (cpvdist(object->GetBody()->p, cpv(840.0f, 525.0f)) > 500.0f) {
        // Stop Velocity
        object->GetBody()->v = cpvzero;
        object->GetBody()->p = LastPos;
    }
    else {
        LastPos =cpv(840.0f, 525.0f);//object->GetBody()->p;
    }

}

void TPhysicsSystem::Initialize()
{
    // Set which components we want to deal with
    Esc::TSystem::HandleComponent("PhysicalObject", true);
}

/******************************************************************/
TRenderingSystem::TRenderingSystem(sf::RenderWindow *pApp) :
  Esc::TSystem(), mpApp(pApp)
{

}
TRenderingSystem::~TRenderingSystem()
{

}

void TRenderingSystem::Update(Esc::TEntityPtr entity, uint32_t tickDelta)
{
    TPhysicalObject *physicalObject =
      static_cast<TPhysicalObject*>(entity->GetComponent("PhysicalObject"));
    TGraphicsObject *graphicObject =
      static_cast<TGraphicsObject*>(entity->GetComponent("GraphicsObject"));

    graphicObject->GetSprite()->SetPosition((int)physicalObject->GetBody()->p.x,
                          (int)physicalObject->GetBody()->p.y);
    graphicObject->GetSprite()->SetRotation(physicalObject->GetBody()->a*(180.0f/PI));
    mpApp->Draw(*(graphicObject->GetSprite()));

}
void TRenderingSystem::Initialize()
{
    // Set which components we want to deal with
    Esc::TSystem::HandleComponent("PhysicalObject", true);
    Esc::TSystem::HandleComponent("GraphicsObject", true);
}

/******************************************************************/
TInputSystem::TInputSystem(sf::RenderWindow *pApp) :
  Esc::TSystem(), mpApp(pApp)
{
    mCenterX = mpApp->GetWidth()/2;
    mCenterY = mpApp->GetHeight()/2;
}
TInputSystem::~TInputSystem()
{

}

void TInputSystem::Update(Esc::TEntityPtr entity, uint32_t tickDelta)
{
    TPhysicalObject *physicalObject =
      static_cast<TPhysicalObject*>(entity->GetComponent("PhysicalObject"));
    const sf::Input& Input = mpApp->GetInput();
    cpFloat speed = physicalObject->GetMaxSpeed();

    // find location of Mouse and point player towards it
    cpFloat MouseX = (cpFloat)Input.GetMouseX();
    cpFloat MouseY = (cpFloat)Input.GetMouseY();

    cpFloat Angle = atan2(MouseY - physicalObject->GetBody()->p.y,
                          MouseX - physicalObject->GetBody()->p.x);
    physicalObject->GetBody()->a = Angle;

#if 0
    cpFloat xForce = 0, yForce = 0;
    bool bChangeVelocity = false;
    cpVect &Vel = physicalObject->GetBody()->v;

    if (Utilities::KeyIsDown(Input, GAME_KEY_LEFT))
        xForce -= speed*20;
    else if (Vel.x < -0.01f) {
        Vel.x = 0;
        bChangeVelocity = true;
    }

    if (Utilities::KeyIsDown(Input, GAME_KEY_RIGHT))
        xForce += speed*20;
    else if (Vel.x > 0.01f) {
        Vel.x = 0;
        bChangeVelocity = true;
    }

    if (Utilities::KeyIsDown(Input, GAME_KEY_DOWN))
        yForce += speed*20;
    else if (Vel.y > 0.01f) {
        Vel.y = 0;
        bChangeVelocity = true;
    }

    if (Utilities::KeyIsDown(Input, GAME_KEY_UP))
        yForce -= speed*20;
    else if (Vel.y < -0.01f) {
        Vel.y = 0;
        bChangeVelocity = true;
    }

    if (bChangeVelocity) {
        physicalObject->GetBody()->v = Vel;
    }

    physicalObject->GetBody()->f = cpv(xForce, yForce);
#else
    if (Utilities::KeyIsDown(Input, GAME_KEY_UP)) {
        physicalObject->GetBody()->f.x = cos(Angle)*speed;
        physicalObject->GetBody()->f.y = sin(Angle)*speed;
    }
    else if (Utilities::KeyIsDown(Input, GAME_KEY_FIRE)) {
        physicalObject->GetBody()->f.x = cos(Angle)*2*speed;
        physicalObject->GetBody()->f.y = sin(Angle)*2*speed;
    }
    else {
        physicalObject->GetBody()->f = cpvzero;
    }
    if (Utilities::KeyIsDown(Input, GAME_KEY_DOWN)) {
        // Slow it down
        physicalObject->GetBody()->v.x = physicalObject->GetBody()->v.x + -physicalObject->GetBody()->v.x/16;
        physicalObject->GetBody()->v.y = physicalObject->GetBody()->v.y + -physicalObject->GetBody()->v.y/16;
    }

#endif

}

void TInputSystem::Initialize()
{
    // Set which components we want to deal with
    Esc::TSystem::HandleComponent("PhysicalObject", true);
    Esc::TSystem::HandleComponent("Input", true);
}

/******************************************************************/
TEnemySystem::TEnemySystem() :
  Esc::TSystem()
{
    World = Esc::TWorld::GetInstance();
}
TEnemySystem::~TEnemySystem()
{

}

void TEnemySystem::Update(Esc::TComponentPtr component, uint32_t tickDelta)
{
    Esc::TEntityPtr Entity = component->GetOwnerEntity();
    Esc::TEntityPtrs playerEntities;

    if (World->GetGroupEntities("Player", playerEntities)) {
        TPhysicalObject *physicalObject =
          static_cast<TPhysicalObject*>(playerEntities[0]->GetComponent("PhysicalObject"));

        cpVect playerLoc = physicalObject->GetBody()->p;

        physicalObject =
          static_cast<TPhysicalObject*>(Entity->GetComponent("PhysicalObject"));

        cpVect enemyLoc = physicalObject->GetBody()->p;
        cpFloat Angle = atan2(playerLoc.y - enemyLoc.y,
                              playerLoc.x - enemyLoc.x);

        // add randomization to angle
        cpFloat RandomDegrees = (cpFloat)(((int32_t)rand() % 20) - 10);
        Angle += (RandomDegrees*PI)/180.0f;

        physicalObject->GetBody()->a = Angle;

        cpVect NewForce, OldForce;
        NewForce = cpv(150*cos(Angle), 150*sin(Angle));
        OldForce = physicalObject->GetBody()->f;

        if ((rand() % 4) == 0)
        {
            cpVect &Vel = physicalObject->GetBody()->v;

            // only reset velocity if force has large difference
            if (abs(NewForce.x - OldForce.x) > 150/2)
            {
                Vel.x = 0;
            }
            if (abs(NewForce.y - OldForce.y) > 150/2)
            {
                Vel.y = 0;
            }
            physicalObject->GetBody()->v = Vel;
        }
        physicalObject->GetBody()->f = NewForce;
    }

}

void TEnemySystem::Initialize()
{
    Esc::TSystem::HandleComponent("Enemy", true);
}

/******************************************************************/
TDamageSystem::TDamageSystem() :
  Esc::TSystem()
{

}
TDamageSystem::~TDamageSystem()
{

}

void TDamageSystem::Update(Esc::TEntityPtr entity, uint32_t tickDelta)
{
    printf("Still in here\n");
    // Check if this is 1st touch, and subtract damage then
    TCollision *collisionComp = static_cast<TCollision*>(entity->GetComponent("Collision"));

    if (collisionComp->IsNewCollision()) {
        Esc::TEntity *pOtherEntity;
        collisionComp->GetNewCollisionEntity((pOtherEntity));

        TDamage *damageComp = static_cast<TDamage*>(pOtherEntity->GetComponent("Damage"));
        // Ensure the thing we hit has a damage entity
        if (damageComp) {
            THealth *healthComp = static_cast<THealth*>(entity->GetComponent("Health"));

            printf("Hit! Health %d, Damage %d\n", healthComp->GetHealth(), damageComp->GetDamage());

            healthComp->AddDamage(damageComp->GetDamage());
        }
    }

}

void TDamageSystem::Initialize()
{
    // Set which components we want to deal with
    Esc::TSystem::HandleComponent("Collision", true);
    Esc::TSystem::HandleComponent("Health", true);
}

/******************************************************************/
TDeathSystem::TDeathSystem(cpSpace *pSpace) :
  Esc::TSystem(), mpSpace(pSpace)
{
    World = Esc::TWorld::GetInstance();
}

TDeathSystem::~TDeathSystem()
{

}

void TDeathSystem::Update(Esc::TComponentPtr component, uint32_t tickDelta)
{
    THealth *health = static_cast<THealth*>(component);

    if (health->IsDead()) {
        printf("Entity Dead! Swap\n");

        // Let's try and swap some components
        Esc::TEntityPtr healthEntity = component->GetOwnerEntity();

        // Find the enemy connected to us via the Collision component
        TCollision *collisionComp = static_cast<TCollision*>(healthEntity->GetComponent("Collision"));

        Esc::TEntityPtrs entities;
        collisionComp->GetEntitiesColliding(entities);
        Esc::TEntityPtr damageEntity = entities[0];

        health->AddHealth(100);

        World->SetGroup(healthEntity, "Player", false);

        Esc::TComponentPtr healthComp = component;
        Esc::TComponentPtr inputComp = static_cast<TInput*>(healthEntity->GetComponent("Input"));
        Esc::TComponentPtr damageComp = static_cast<TDamage*>(damageEntity->GetComponent("Damage"));
        Esc::TComponentPtr enemyComp = static_cast<TEnemy*>(damageEntity->GetComponent("Enemy"));

        World->RemoveComponent(healthEntity, healthComp, false);
        World->RemoveComponent(healthEntity, inputComp, false);
        World->RemoveComponent(damageEntity, damageComp, false);
        World->RemoveComponent(damageEntity, enemyComp, false);

        World->AddComponent(damageEntity, healthComp);
        World->AddComponent(damageEntity, inputComp);
        World->AddComponent(healthEntity, damageComp);
        World->AddComponent(healthEntity, enemyComp);

        World->SetGroup(damageEntity, "Player", true);

        //World->DeleteEntity(healthEntity);


    }
}

void TDeathSystem::Initialize()
{
    Esc::TSystem::HandleComponent("Health", true);
}

/******************************************************************************
*
* CheckGameEnd() - Check if the user hit X or Escape
*
******************************************************************************/
bool Game::CheckGameEnd()
{
    bool bEndGame = false;
    sf::Event Event;

    while (pApp->PollEvent(Event))
    {
        // Window closed
        if (Event.Type == sf::Event::Closed)
        {
            bEndGame = true;
            break;
        }

        // Escape key pressed
        if (Event.Type == sf::Event::KeyPressed)
        {
            if (Event.Key.Code == sf::Key::Escape)
            {
                bEndGame = true;
                break;
            }
        }
    }

    return bEndGame;
}

#if 0
/******************************************************************************
*
* DrawStatusUpdate() - Write the Player's status on the Screen
*
******************************************************************************/
void DrawStatusUpdate(sf::RenderWindow *pApp, SmashPcPlayer *pOurPlayer,
                      SmashPcWeapon *pWeapon, U32 u32Lives,
                      U32 u32ScreenX, U32 u32ScreenY)
{
    sf::Text InfoText;

    InfoText.SetFont(sf::Font::GetDefaultFont());
    InfoText.SetCharacterSize(30);
    InfoText.SetPosition(u32ScreenX/4, u32ScreenY - 90);
    InfoText.SetColor(sf::Color(255, 255, 255, 200));

    stringstream ss;
    ss << "Lives: " << u32Lives << " THealth: " << 3;//pOurPlayer->mu32THealth;
    ss << " Weapons Available: ";

    string Info = ss.str();

    U32 u32Bullet = 0;
    while(u32Bullet == 0)//pWeapon->IsBulletAvailable(u32Bullet) == SmashPcWeapon::AVAILABLE)
    {
        stringstream ss;

        ss << u32Bullet+1;
        Info += ss.str() + " ";
        u32Bullet++;
    }

    InfoText.SetString(sf::String(Info.c_str()));
    pApp->Draw(InfoText);
}
#endif
