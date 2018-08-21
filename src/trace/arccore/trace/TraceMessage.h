/*---------------------------------------------------------------------------*/
/* TraceMessage.h                                              (C) 2000-2018 */
/*                                                                           */
/* Message de trace.                                                         */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_TRACE_TRACEMESSAGE_H
#define ARCCORE_TRACE_TRACEMESSAGE_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/trace/Trace.h"

#include <iostream>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arccore
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Gestion d'un message.
 *
 Cette classe se g�re comme un flot de sortie standard (ostream&) et
 permet d'envoyer un message du type sp�cifi� par #eTraceMessageClass.
 
 \warning Les instances de cette classe sont normalement cr��es par
 un gestionnaire de message ITraceMng.
*/
class ARCCORE_TRACE_EXPORT TraceMessage
{
 public:
  static const int DEFAULT_LEVEL = Trace::DEFAULT_VERBOSITY_LEVEL;
 public:
  TraceMessage(std::ostream*,ITraceMng*,Trace::eMessageType,int level =DEFAULT_LEVEL);
  TraceMessage(const TraceMessage& from);
  const TraceMessage& operator=(const TraceMessage& from);
  ~TraceMessage() ARCCORE_NOEXCEPT_FALSE;

 public:
  std::ostream& file() const;
  const TraceMessage& width(Integer v) const;
  ITraceMng* parent() const { return m_parent; }
  Trace::eMessageType type() const { return m_type; }
  int level() const { return m_level; }
  int color() const { return m_color; }
 private:
  std::ostream* m_stream; //!< Flot sur lequel le message est envoy�
  ITraceMng* m_parent; //!< Gestionnaire de message parent
  Trace::eMessageType m_type; //!< Type de message
  int m_level; //!< Niveau du message
 public:
  mutable int m_color; //!< Couleur du message.
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Trace
{
ARCCORE_TRACE_EXPORT std::ostream&
operator<< (std::ostream& o,const Width& w);

ARCCORE_TRACE_EXPORT std::ostream&
operator<< (std::ostream& o,const Precision& w);
}

#ifndef ARCCORE_DEBUG
class ARCCORE_TRACE_EXPORT TraceMessageDbg
{
};
template<class T> inline const TraceMessageDbg&
operator<<(const TraceMessageDbg& o,const T&)
{
  return o;
}
#endif

ARCCORE_TRACE_EXPORT const TraceMessage&
operator<<(const TraceMessage& o,const Trace::Color& c);

template<class T> inline const TraceMessage&
operator<<(const TraceMessage& o,const T& v)
{
  o.file() << v;
  return o;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arccore

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif

