/**
 * @file planet.cpp
 * @date 19-Mar-2024
 */

#define __SCROLL_IMPL__
#include "planet.h"
#undef __SCROLL_IMPL__

#include "Object.h"
#include "orxExtensions.h"

#ifdef __orxMSVC__

/* Requesting high performance dedicated GPU on hybrid laptops */
__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#endif // __orxMSVC__

/** Update function, it has been registered to be called every tick of the core clock
 */
void planet::Update(const orxCLOCK_INFO &_rstInfo)
{
  // Should quit?
  if (orxInput_IsActive("Quit"))
  {
    // Send close event
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
  }
}

/** Init function, it is called when all orx's modules have been initialized
 */
orxSTATUS planet::Init()
{
  // Init extensions
  InitExtensions();

  // Create the scene
  CreateObject("Startup");

  // Done!
  return orxSTATUS_SUCCESS;
}

/** Run function, it should not contain any game logic
 */
orxSTATUS planet::Run()
{
  // Return orxSTATUS_FAILURE to instruct orx to quit
  return orxSTATUS_SUCCESS;
}

/** Exit function, it is called before exiting from orx
 */
void planet::Exit()
{
  // Exit from extensions
  ExitExtensions();

  // Let orx clean all our mess automatically. :)
}

/** BindObjects function, ScrollObject-derived classes are bound to config sections here
 */
void planet::BindObjects()
{
  // Bind Scroll classes to matching config sections
  ScrollBindObject<game::Dropper>("Dropper");
  ScrollBindObject<Object>("Object");
  ScrollBindObject<game::Planet>("Planet");
}

/** Bootstrap function, it is called before config is initialized, allowing for early resource storage definitions
 */
orxSTATUS planet::Bootstrap() const
{
  // Bootstrap extensions
  BootstrapExtensions();

  // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
  return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
  // Execute our game
  planet::GetInstance().Execute(argc, argv);

  // Done!
  return EXIT_SUCCESS;
}
