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
  auto xDirection = orxInput_GetValue("Right") - orxInput_GetValue("Left");
  orxVECTOR speed = {xDirection, 0, 0};
  orxVector_Mulf(&speed, &speed, 100);
  SetSpeed(speed);

  if (orxInput_HasBeenActivated("Drop"))
  {
    orxVECTOR position = orxVECTOR_0;
    GetPosition(position, orxTRUE);

    PushConfigSection();
    auto planet = orxObject_CreateFromConfig(orxConfig_GetString("Drop"));
    PopConfigSection();
    orxObject_SetWorldPosition(planet, &position);
  }

  Object::Update(_rstInfo);
}
