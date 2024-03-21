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

    return orxString_Compare(a->GetModelName(), b->GetModelName()) == 0;
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
  Object::Update(_rstInfo);
}

void game::Planet::OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal)
{
  if (!EqualModelName(this, _poCollider))
  {
    // Nothing to do if this isn't a planet to planet collision
    return;
  }

  PushConfigSection();
  auto stay = orxConfig_GetBool("Stay");
  PopConfigSection();

  if (GetLifeTime() != 0 && !stay)
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
  const orxFLOAT MIN_DROP_WAIT_TIME = 1.0;

  // Create a planet if it's been long enough since we dropped one
  if (!latest)
  {
    dtSinceDrop += _rstInfo.fDT;
    if (dtSinceDrop > MIN_DROP_WAIT_TIME)
    {
      dtSinceDrop = 0.0;
      CreatePlanet();
    }

    return;
  }

  // Movement
  auto xDirection = orxInput_GetValue("Right") - orxInput_GetValue("Left");
  orxVECTOR speed = {xDirection, 0, 0};
  orxVector_Mulf(&speed, &speed, 300);
  orxObject_SetSpeed(latest, &speed);
  // SetSpeed(speed);

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
