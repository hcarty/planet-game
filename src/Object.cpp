/**
 * @file Object.cpp
 * @date 19-Mar-2024
 */

#include <map>

#include "Object.h"

// General game functionality

// Event handling

namespace game::event
{
  // Key holding names of sections with event handler definitions for an object
  auto CONFIG_KEY = "EventHandlerList";
  auto COMMAND = "Event.Send";

  // Map from event name to registered handlers listening for that event
  static std::map<orxSTRINGID, std::map<const orxOBJECT *, orxSTRINGID>> eventRecipientObjects{};

  void Command(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
  {
    switch (_u32ArgNumber)
    {
    case 1ul:
    {
      // Event name
      Send(_astArgList[0].zValue);
      break;
    }
    case 2ul:
    {
      // Event name and a recipient object
      auto object = orxOBJECT(orxStructure_Get(_astArgList[1].u64Value));
      if (object)
      {
        Send(_astArgList[0].zValue, object);
      }
      break;
    }
    default:
    {
      // This should not be possible if the command is registered properly
      orxASSERT(orxFALSE);
      break;
    }
    }
  }

  // Report input events as game events
  orxSTATUS InputEventHandler(const orxEVENT *event)
  {
    // This handler should only be called for input events
    orxASSERT(event->eType == orxEVENT_TYPE_INPUT);

    switch (event->eID)
    {
    case orxINPUT_EVENT_ON:
    {
      orxINPUT_EVENT_PAYLOAD *input = static_cast<orxINPUT_EVENT_PAYLOAD *>(event->pstPayload);
      orxCHAR eventName[64] = "";
      orxString_NPrint(eventName, sizeof(eventName), "Input:%s:%s", input->zSetName, input->zInputName);
      Send(eventName);
      break;
    }
    default:
    {
      // This should not happen as long as we register the event handler correctly
      orxLOG("Unhandled input event ID");
      orxASSERT(orxFALSE);
    }
    }

    return orxSTATUS_SUCCESS;
  }

  // Record events handled by newly created objects and removes objects when they're deleted
  orxSTATUS ObjectEventHandler(const orxEVENT *event)
  {
    // This handler should only be called for object events
    orxASSERT(event->eType == orxEVENT_TYPE_OBJECT);

    switch (event->eID)
    {
    case orxOBJECT_EVENT_CREATE:
    {
      // The object being created
      orxOBJECT *obj = static_cast<orxOBJECT *>(event->hSender);
      orxASSERT(obj);

      // Get the object's name and therefore config section
      auto name = orxObject_GetName(obj);
      if (!orxConfig_HasSection(name))
      {
        // No config section matching this object
        break;
      }

      // Get supported events from config
      orxConfig_PushSection(name);
      {
        auto sections = orxConfig_GetListCount(CONFIG_KEY);
        for (int i = 0; i < sections; i++)
        {
          // Process event handlers from given section
          auto section = orxConfig_GetListString(CONFIG_KEY, i);
          if (!orxConfig_HasSection(section))
          {
            orxLOG("No section %s referenced in %s from section %s", section, CONFIG_KEY, name);
            continue;
          }

          // Register all handlers according to their keys
          orxConfig_PushSection(section);
          {
            auto keys = orxConfig_GetKeyCount();
            for (orxU32 j = 0; j < keys; j++)
            {
              auto key = orxConfig_GetKey(j);
              auto id = orxString_GetID(key);

              // Add the current object to the set of recipients with handlers for this event
              eventRecipientObjects[id][obj] = orxString_GetID(section);
            }
          }
          orxConfig_PopSection();
        }
      }
      orxConfig_PopSection();
      break;
    }
    case orxOBJECT_EVENT_DELETE:
    {
      orxOBJECT *obj = static_cast<orxOBJECT *>(event->hSender);
      orxASSERT(obj);

      // Get the object's name and therefore config section
      auto name = orxObject_GetName(obj);
      if (!orxConfig_HasSection(name))
      {
        // No config section matching this object
        break;
      }

      // Get supported events from config
      orxConfig_PushSection(name);
      {
        auto sections = orxConfig_GetListCount(CONFIG_KEY);
        for (int i = 0; i < sections; i++)
        {
          // Process event handlers from given section
          auto section = orxConfig_GetListString(CONFIG_KEY, i);
          if (!orxConfig_HasSection(section))
          {
            orxLOG("No section %s referenced in %s from section %s", section, CONFIG_KEY, name);
            continue;
          }

          // Register all handlers according to their keys
          orxConfig_PushSection(section);
          {
            auto keys = orxConfig_GetKeyCount();
            for (orxU32 j = 0; j < keys; j++)
            {
              auto key = orxConfig_GetKey(j);
              auto id = orxString_GetID(key);

              // Remove the current object from the set of recipients with handlers for this event
              eventRecipientObjects[id].erase(obj);
            }
          }
          orxConfig_PopSection();
        }
      }
      orxConfig_PopSection();
      break;
    }
    default:
    {
      // This should not happen as long as we register the event handler correctly
      orxLOG("Unhandled object event ID");
      orxASSERT(orxFALSE);
    }
    };

    return orxSTATUS_SUCCESS;
  }

  void Init()
  {
    // Object event handling
    orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, ObjectEventHandler);
    orxEvent_SetHandlerIDFlags(ObjectEventHandler, orxEVENT_TYPE_OBJECT, orxNULL,
                               orxEVENT_GET_FLAG(orxOBJECT_EVENT_CREATE) | orxEVENT_GET_FLAG(orxOBJECT_EVENT_DELETE),
                               orxEVENT_KU32_MASK_ID_ALL);

    // Input event handling
    orxEvent_AddHandler(orxEVENT_TYPE_INPUT, InputEventHandler);
    orxEvent_SetHandlerIDFlags(InputEventHandler, orxEVENT_TYPE_INPUT, orxNULL,
                               orxEVENT_GET_FLAG(orxINPUT_EVENT_ON),
                               orxEVENT_KU32_MASK_ID_ALL);

    // Register commands
    orxCOMMAND_REGISTER(COMMAND, Command, "none", orxCOMMAND_VAR_TYPE_NONE, 1, 1, {"Event", orxCOMMAND_VAR_TYPE_STRING}, {"Object = <void>", orxCOMMAND_VAR_TYPE_U64});
  }

  void Exit()
  {
    // Event handling
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, ObjectEventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, InputEventHandler);

    // Commands
    orxCOMMAND_UNREGISTER(COMMAND);
  }

  void EvaluateCommandFromConfig(const orxSTRING section, const orxSTRING key, const orxU64 guid)
  {
    orxConfig_PushSection(section);
    orxCOMMAND_VAR _result;
    orxCommand_EvaluateWithGUID(orxConfig_GetString(key), guid, &_result);
    orxConfig_PopSection();
  }

  // Send an event to all listeners for an event
  void Send(const orxSTRING event)
  {
    auto id = orxString_GetID(event);
    auto recipients = eventRecipientObjects[id];
    for (const auto [object, sectionID] : recipients)
    {
      auto section = orxString_GetFromID(sectionID);
      EvaluateCommandFromConfig(section, event, orxStructure_GetGUID(orxSTRUCTURE(object)));
    }
  }

  // Send an event to a specific object
  void Send(const orxSTRING event, const orxOBJECT *object)
  {
    auto eventID = orxString_GetID(event);
    if (!eventRecipientObjects.contains(eventID) || !eventRecipientObjects[eventID].contains(object))
    {
      orxLOG("Event %s is not registered to any objects", event);
      return;
    }

    auto sectionID = eventRecipientObjects[eventID][object];
    EvaluateCommandFromConfig(orxString_GetFromID(sectionID), event, orxStructure_GetGUID(orxSTRUCTURE(object)));
  }
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

void Object::PushInputSet()
{
  PushConfigSection();
  if (orxConfig_HasValueNoCheck("Input"))
  {
    orxInput_PushSet(orxConfig_GetString("Input"));
  }
  PopConfigSection();
}

void Object::PopInputSet()
{
  PushConfigSection();
  if (orxConfig_HasValueNoCheck("Input"))
  {
    orxInput_PopSet();
  }
  PopConfigSection();
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
      // Send an event signaling the Game Over state
      event::Send("GameOver");

      // Reset the timer so that we don't fire the event every frame
      touchingArenaTop.reset();
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

      orxConfig_PushSection(nextPlanet);
      auto scoreDiff = orxConfig_GetU32("Score");
      orxConfig_PopSection();
      orxConfig_PushSection("Runtime");
      auto score = orxConfig_GetU32("Score");
      orxConfig_SetU32("Score", score + scoreDiff);
      orxConfig_PopSection();
    }
    PopConfigSection();
  }
}

void game::Planet::OnArenaTopCollide()
{
  // Start counting time in contact
  touchingArenaTop = 0.0f;
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

void game::Dropper::UpdatePosition(const orxCLOCK_INFO &_rstInfo)
{
  // Movement specs from config
  PushConfigSection();
  const auto minX = orxConfig_GetFloat("MinX");
  const auto maxX = orxConfig_GetFloat("MaxX");
  orxVECTOR speed = orxVECTOR_0;
  orxConfig_GetVector("MaxSpeed", &speed);
  PopConfigSection();

  // Dropper position
  orxVECTOR position = orxVECTOR_0;
  GetPosition(position);

  // User controls
  auto xDirection = orxInput_GetValue("Right") - orxInput_GetValue("Left");
  orxVector_Mulf(&speed, &speed, xDirection);
  orxVector_Add(&position, &position, orxVector_Mulf(&speed, &speed, _rstInfo.fDT));

  // Constrain position to playable area
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
}

void game::Dropper::DropPlanet()
{
  // Planet must be present for it to be dropped
  orxASSERT(latest);

  // Get the config name of the current planet
  auto name = orxObject_GetName(latest);

  // Remove placeholder planet object
  orxObject_SetLifeTime(latest, 0);
  // Mark our held object as gone
  latest = orxNULL;

  // Create a replacement "real" object which will drop
  auto planet = orxObject_CreateFromConfig(name);
  orxVECTOR position = orxVECTOR_0;
  GetPosition(position);
  orxObject_SetPosition(planet, &position);
}

void game::Dropper::Update(const orxCLOCK_INFO &_rstInfo)
{
  PushInputSet();

  PushConfigSection();
  const auto minDropWaitTime = orxConfig_GetFloat("MinDropWait");
  PopConfigSection();

  // Dropper movement and position bounds
  UpdatePosition(_rstInfo);

  // Drop current planet if we have one
  if (latest && orxInput_HasBeenActivated("Drop"))
  {
    DropPlanet();
  }

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
  }

  PopInputSet();

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
