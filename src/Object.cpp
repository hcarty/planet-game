/**
 * @file Object.cpp
 * @date 19-Mar-2024
 */

#include "Object.h"

// General game functionality

// Event handling

void game::SendEvent(game::Event event)
{
  orxEvent_SendShort(orxEVENT_TYPE_USER_DEFINED, orxENUM(event));
}

void game::RegisterEventHandler(Event id, orxEVENT_HANDLER handler)
{
  orxEvent_AddHandler(orxEVENT_TYPE_USER_DEFINED, handler);
  orxEvent_SetHandlerIDFlags(handler, orxEVENT_TYPE_USER_DEFINED, orxNULL, orxEVENT_GET_FLAG(id), orxEVENT_KU32_MASK_ID_ALL);
}

void game::RemoveEventHandler(Event id, orxEVENT_HANDLER handler)
{
  orxEvent_RemoveHandler(orxEVENT_TYPE_USER_DEFINED, handler);
}

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
      orxConfig_PushSection("Runtime");
      auto scene = orxOBJECT(orxStructure_Get(orxConfig_GetU64("Scene")));
      orxConfig_PopSection();
      orxObject_SetLifeTime(scene, 0);
    }

    SendEvent(Event::GameOver);
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
  PushConfigSection();
  const auto minDropWaitTime = orxConfig_GetFloat("MinDropWait");
  const auto minX = orxConfig_GetFloat("MinX");
  const auto maxX = orxConfig_GetFloat("MaxX");
  orxVECTOR speed = orxVECTOR_0;
  orxConfig_GetVector("MaxSpeed", &speed);
  PopConfigSection();

  // Dropper position
  orxVECTOR position = orxVECTOR_0;
  GetPosition(position);

  // Dropper movement and position bounds
  // User controls
  auto xDirection = orxInput_GetValue("Right") - orxInput_GetValue("Left");
  orxVector_Mulf(&speed, &speed, xDirection);
  orxVector_Add(&position, &position, orxVector_Mulf(&speed, &speed, _rstInfo.fDT));
  if (position.fX < minX)
  {
    position.fX = minX;
  }
  else if (position.fX > maxX)
  {
    position.fX = maxX;
  }
  // Set updated position
  SetPosition(position);

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

  // Drop current planet if we have one
  if (latest && orxInput_HasBeenActivated("Drop"))
  {
    // Get the config name of the current planet
    auto name = orxObject_GetName(latest);

    // Remove placeholder planet object
    orxObject_SetLifeTime(latest, 0);
    // Mark our held object as gone
    latest = orxNULL;

    // Create a replacement "real" object which will drop
    auto planet = orxObject_CreateFromConfig(name);
    orxObject_SetWorldPosition(planet, &position);
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

  // Remove physics body so we can safely set this as a child object
  orxObject_UnlinkStructure(latest, orxSTRUCTURE_ID_BODY);

  orxObject_SetParent(latest, GetOrxObject());
}
