/*---------------------------------------------------------------------------*/
/* MessagePassingMng.cc                                        (C) 2000-2018 */
/*                                                                           */
/* Gestionnaire des échanges de messages.                                    */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/messagepassing/MessagePassingGlobal.h"
#include "arccore/messagepassing/MessagePassingMng.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arccore
{

namespace MessagePassing
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

MessagePassingMng::
MessagePassingMng(IDispatchers* d)
: m_dispatchers(d)
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

MessagePassingMng::
~MessagePassingMng()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

IDispatchers* MessagePassingMng::
dispatchers()
{
  return m_dispatchers;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace MessagePassing

} // End namespace Arccore

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
