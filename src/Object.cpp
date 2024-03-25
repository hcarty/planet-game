/**
 * @file Object.cpp
 * @date 19-Mar-2024
 */

#include "Object.h"

// Generic objects

void Object::OnCreate()
{
  orxConfig_SetBool("IsObject", orxTRUE);
}

void Object::OnDelete()
{
}

void Object::Update(const orxCLOCK_INFO &_rstInfo)
{
}

// Planets

namespace
{
  bool EqualContent(const orxSTRING a, const orxSTRING b)
  {
    return orxString_Compare(a, b) == 0;
  }

  /// @brief Check if object model (config) names are equal
  /// @param a First object to compare
  /// @param b Second object to compare
  /// @return `true` if `a` and `b` are not NULL and have the same config name, otherwise `false`
  bool EqualModelName(ScrollObject *a, ScrollObject *b)
  {
    if (a == orxNULL || b == orxNULL)
    {
      return false;
    }

    return EqualContent(a->GetModelName(), b->GetModelName());
  }
}

void game::Planet::OnCreate()
{
  Object::OnCreate();
}

void game::Planet::OnDelete()
{
  Object::OnDelete();
}

void game::Planet::Update(const orxCLOCK_INFO &_rstInfo)
{
  // Add time if we're in contact with the arena top
  if (touchingArenaTop.has_value())
  {
    touchingArenaTop = *touchingArenaTop + _rstInfo.fDT;
    if (*touchingArenaTop > 2.0)
    {
      orxLOG("TODO: Set game over");
      SetLifeTime(0);
    }
  }

  Object::Update(_rstInfo);
}

void game::Planet::OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
  if (EqualModelName(this, _poCollider))
  {
    OnPlanetCollide(_poCollider);
  }

  if (EqualContent(_poCollider->GetModelName(), "ArenaTop"))
  {
    OnArenaTopCollide();
  }
}

void game::Planet::OnSeparate(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart)
{
  if (EqualContent(_poCollider->GetModelName(), "ArenaTop"))
  {
    OnArenaTopSeparate();
  }
}

void game::Planet::OnPlanetCollide(ScrollObject *_poCollider)
{
  PushConfigSection();
  auto stay = orxConfig_GetBool("Stay");
  PopConfigSection();

  if (!stay && GetLifeTime() != 0 && _poCollider->GetLifeTime() != 0)
  {
    // Both objects go away!
    SetLifeTime(0);
    _poCollider->SetLifeTime(0);

    // Get the midpoint of the two objects' locations
    orxVECTOR pos, colliderPos;
    GetPosition(pos, orxTRUE);
    _poCollider->GetPosition(colliderPos, orxTRUE);
    orxVector_Add(&pos, &pos, &colliderPos);
    orxVector_Divf(&pos, &pos, 2);

    // Create a new planet at the midpoint!
    PushConfigSection();
    if (orxConfig_HasValue("Next"))
    {
      auto nextPlanet = orxConfig_GetString("Next");
      auto planet = orxObject_CreateFromConfig(nextPlanet);
      orxObject_SetWorldPosition(planet, &pos);
    }
    PopConfigSection();
  }
}

void game::Planet::OnArenaTopCollide()
{
  // Start counting time in contact
  touchingArenaTop = 0.0;
}

void game::Planet::OnArenaTopSeparate()
{
  // Clear current counter
  touchingArenaTop.reset();
}

void game::Dropper::OnCreate()
{
  Object::OnCreate();
}

void game::Dropper::OnDelete()
{
  Object::OnDelete();
}

void game::Dropper::Update(const orxCLOCK_INFO &_rstInfo)
{
  orxVECTOR speed = orxVECTOR_0;

  PushConfigSection();
  const auto minDropWaitTime = orxConfig_GetFloat("MinDropWait");
  const auto minX = orxConfig_GetFloat("MinX");
  const auto maxX = orxConfig_GetFloat("MaxX");
  orxVECTOR leftSpeed = orxVECTOR_0;
  orxVECTOR rightSpeed = orxVECTOR_0;
  orxConfig_GetVector("MaxSpeed", &rightSpeed);
  orxVector_Mulf(&leftSpeed, &rightSpeed, -1);
  PopConfigSection();

  // Dropper position
  orxVECTOR position = orxVECTOR_0;
  GetPosition(position, orxTRUE);

  // Dropper movement
  if (position.fX < minX)
  {
    // Stay inside bounds
    speed = rightSpeed;
  }
  else if (position.fX > maxX)
  {
    // Stay inside bounds
    speed = leftSpeed;
  }
  else
  {
    // User controls
    auto xDirection = orxInput_GetValue("Right") - orxInput_GetValue("Left");
    orxVector_Mulf(&speed, &rightSpeed, xDirection);
  }
  SetSpeed(speed);

  // Create a planet if it's been long enough since we dropped one
  if (!latest)
  {

    dtSinceDrop += _rstInfo.fDT;
    if (first || dtSinceDrop > minDropWaitTime)
    {
      first = false;
      dtSinceDrop = 0.0;
      CreatePlanet();
    }

    return;
  }

  // Movement
  orxObject_SetSpeed(latest, &speed);

  // Drop current planet if we have one
  if (latest && orxInput_HasBeenActivated("Drop"))
  {
    orxConfig_PushSection(orxObject_GetName(latest));
    orxConfig_PushSection(orxConfig_GetString("Body"));
    orxVECTOR gravity = orxVECTOR_0;
    orxConfig_GetVector("CustomGravity", &gravity);
    orxConfig_PopSection();
    orxConfig_PopSection();

    orxObject_SetCustomGravity(latest, &gravity);
    orxObject_SetSpeed(latest, &orxVECTOR_0);
    latest = orxNULL;
  }

  Object::Update(_rstInfo);
}

void game::Dropper::CreatePlanet()
{
  orxASSERT(latest == orxNULL);

  orxVECTOR position = orxVECTOR_0;
  GetPosition(position, orxTRUE);

  PushConfigSection();
  latest = orxObject_CreateFromConfig(orxConfig_GetString("Drop"));
  PopConfigSection();
  orxObject_SetWorldPosition(latest, &position);

  orxObject_SetCustomGravity(latest, &orxVECTOR_0);
}
