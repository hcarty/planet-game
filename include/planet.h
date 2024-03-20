/**
 * @file planet.h
 * @date 19-Mar-2024
 */

#ifndef __planet_H__
#define __planet_H__

#define __NO_SCROLLED__
#include "Scroll.h"

/** Game Class
 */
class planet : public Scroll<planet>
{
public:


private:

                orxSTATUS       Bootstrap() const;

                void            Update(const orxCLOCK_INFO &_rstInfo);

                orxSTATUS       Init();
                orxSTATUS       Run();
                void            Exit();
                void            BindObjects();


private:
};

#endif // __planet_H__
