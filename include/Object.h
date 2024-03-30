#pragma once

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

private:
};

namespace game
{
  enum class Event : orxENUM
  {
    GameOver,
  };

  void SendEvent(Event event);
  void RegisterEventHandler(Event id, orxEVENT_HANDLER handler);
  void RemoveEventHandler(Event id, orxEVENT_HANDLER handler);

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

    void OnPlanetCollide(ScrollObject *_poCollider);
    void OnArenaTopCollide();
    void OnArenaTopSeparate();
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
