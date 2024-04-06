#pragma once

#include <map>
#include <optional>

#include "planet.h"

/** Object Class
 */
class Object : public ScrollObject
{
public:
protected:
  void OnCreate();
  void OnDelete();
  void Update(const orxCLOCK_INFO &_rstInfo);

  void PushInputSet();
  void PopInputSet();

private:
};

namespace game
{
  namespace event
  {
    // Call at init time to setup event handling and commands
    void Init();

    // Call at exit time to remove event handlers and commands
    void Exit();

    // Send an event to all listeners for an event
    void Send(const orxSTRING event);

    // Send an event to a specific object
    void Send(const orxSTRING event, const orxOBJECT *object);
  }

  class Planet : public Object
  {
  public:
  protected:
    void OnCreate();
    void OnDelete();
    void Update(const orxCLOCK_INFO &_rstInfo);
    void OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);
    void OnSeparate(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart);

  private:
    std::optional<orxFLOAT> touchingArenaTop{};

    // Engine event handlers

    void OnPlanetCollide(ScrollObject *_poCollider);
    void OnArenaTopCollide();
    void OnArenaTopSeparate();

    // Game event handlers

    // static orxSTATUS OnGameOver(const orxEVENT *event);
  };

  class Dropper : public Object
  {
  public:
  protected:
    void OnCreate();
    void OnDelete();
    void Update(const orxCLOCK_INFO &_rstInfo);

  private:
    void UpdatePosition(const orxCLOCK_INFO &_rstInfo);

    void CreatePlanet();
    void DropPlanet();

    bool first{true};
    orxOBJECT *latest{orxNULL};
    orxFLOAT dtSinceDrop{0.0};
  };
}
