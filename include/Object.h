#pragma once

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
  class Planet : public Object
  {
  public:
  protected:
    void OnCreate();
    void OnDelete();
    void Update(const orxCLOCK_INFO &_rstInfo);
    void OnCollide(ScrollObject *_poCollider, orxBODY_PART *_pstPart, orxBODY_PART *_pstColliderPart, const orxVECTOR &_rvPosition, const orxVECTOR &_rvNormal);

  private:
  };

  class Dropper : public Object
  {
  public:
  protected:
    void OnCreate();
    void OnDelete();
    void Update(const orxCLOCK_INFO &_rstInfo);

  private:
  };
}
