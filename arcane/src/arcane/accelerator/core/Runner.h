﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2022 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Runner.h                                                    (C) 2000-2022 */
/*                                                                           */
/* Gestion de l'exécution sur accélérateur.                                  */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_ACCELERATOR_CORE_RUNNER_H
#define ARCANE_ACCELERATOR_CORE_RUNNER_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/Ref.h"
#include "arcane/utils/MemoryRessource.h"
#include "arcane/accelerator/core/RunQueue.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::Accelerator
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Gestionnaire d'exécution pour accélérateur.
 * \warning API en cours de définition.
 *
 * Une instance de cette classe représente un backend d'exécution. Il faut
 * d'abord appelé initialize() avant de pouvoir utiliser les méthodes de
 * l'instance ou alors il faut appeler l'un des constructeurs autre que le
 * constructeur par défaut.
 */
class ARCANE_ACCELERATOR_CORE_EXPORT Runner
{
  friend impl::RunQueueImpl;
  friend impl::RunCommandImpl;
  friend RunQueue;
  friend RunQueueEvent;
  class Impl;

 public:

  /*!
   * \brief Créé un gestionnaire d'exécution non initialisé.
   *
   * Il faudra appeler initialize() avant de pouvoir utiliser l'instance
   */
  Runner();
  //! Créé et initialise un gestionnaire pour l'accélérateur \a p
  explicit Runner(eExecutionPolicy p);
  //! Créé et initialise un gestionnaire pour l'accélérateur \a p et l'accélérateur \a device
  Runner(eExecutionPolicy p,DeviceId device);
  ~Runner();

 public:

  Runner(const Runner&) = delete;
  Runner(Runner&&) = delete;
  Runner& operator=(const Runner&) = delete;
  Runner& operator=(Runner&&) = delete;

 public:

  //! Politique d'exécution associée
  eExecutionPolicy executionPolicy() const;

  //! Initialise l'instance. Cette méthode ne doit être appelée qu'une seule fois.
  void initialize(eExecutionPolicy v);

  //! Initialise l'instance. Cette méthode ne doit être appelée qu'une seule fois.
  void initialize(eExecutionPolicy v,DeviceId device);

  //! Indique si l'instance a été initialisée
  bool isInitialized() const;

  /*!
   * \brief Indique si on autorise la création de RunQueue depuis plusieurs threads.
   *
   * Cela nécessite d'utiliser un verrou (comme std::mutex) et peut dégrader les
   * performances. Le défaut est \a false.
   */
  void setConcurrentQueueCreation(bool v);

  //! Indique si la création concurrent de plusieurs RunQueue est autorisé
  bool isConcurrentQueueCreation() const;

  /*!
   * \brief Temps total passé dans les commandes associées à cette instance.
   *
   * Ce temps n'est significatif que si les RunQueue sont synchrones.
   */
  double cumulativeCommandTime() const;

  //! Positionne la politique d'exécution des réductions
  void setDeviceReducePolicy(eDeviceReducePolicy v);

  //! politique d'exécution des réductions
  eDeviceReducePolicy deviceReducePolicy() const;

  //! Positionne un conseil sur la gestion d'une zone mémoire
  void setMemoryAdvice(MemoryView buffer, eMemoryAdvice advice);

  //! Supprime un conseil sur la gestion d'une zone mémoire
  void unsetMemoryAdvice(MemoryView buffer, eMemoryAdvice advice);

  //! Device associé à cette instance.
  DeviceId deviceId() const;

  /*!
   * \brief Positionne le device associé à cette instance comme le device par défaut du contexte.
   *
   * Cet appel est équivalent à cudaSetDevice() ou hipSetDevice();
   */
  void setAsCurrentDevice();

 private:

  impl::RunQueueImpl* _internalCreateOrGetRunQueueImpl();
  impl::RunQueueImpl* _internalCreateOrGetRunQueueImpl(const RunQueueBuildInfo& bi);
  void _internalFreeRunQueueImpl(impl::RunQueueImpl*);
  impl::IRunQueueEventImpl* _createEvent();
  impl::IRunQueueEventImpl* _createEventWithTimer();
  void _addCommandTime(double v);
  impl::IRunnerRuntime* _internalRuntime() const;

 private:

  Impl* m_p;

 private:

  void _checkIsInit() const;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Créé une file temporaire associée à \a runner.
 *
 * Cet appel est thread-safe si runner.isConcurrentQueueCreation()==true.
 */
inline RunQueue
makeQueue(Runner& runner)
{
  return RunQueue(runner);
}

/*!
 * \brief Créé une file temporaire associée à \a runner.
 *
 * Cet appel est thread-safe si runner.isConcurrentQueueCreation()==true.
 */
inline RunQueue
makeQueue(Runner* runner)
{
  ARCANE_CHECK_POINTER(runner);
  return RunQueue(*runner);
}

/*!
 * \brief Créé une file temporaire associée à \a runner avec les propriétés \a bi.
 *
 * Cet appel est thread-safe si runner.isConcurrentQueueCreation()==true.
 */
inline RunQueue
makeQueue(Runner& runner,const RunQueueBuildInfo& bi)
{
  return RunQueue(runner,bi);
}

/*!
 * \brief Créé une file temporaire associée à \a runner avec les propriétés \a bi.
 *
 * Cet appel est thread-safe si runner.isConcurrentQueueCreation()==true.
 */
inline RunQueue
makeQueue(Runner* runner,const RunQueueBuildInfo& bi)
{
  ARCANE_CHECK_POINTER(runner);
  return RunQueue(*runner,bi);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Créé une référence sur file avec la politique d'exécution par défaut de \a runner.
 *
 * Si la file est temporaire, il est préférable d'utiliser makeQueue() à la place
 * pour éviter une allocation inutile.
 */
inline Ref<RunQueue>
makeQueueRef(Runner& runner)
{
  return makeRef(new RunQueue(runner));
}

/*!
 * \brief Créé une référence sur file avec la politique d'exécution par défaut de \a runner.
 *
 * Si la file est temporaire, il est préférable d'utiliser makeQueue() à la place
 * pour éviter une allocation inutile.
 */
inline Ref<RunQueue>
makeQueueRef(Runner& runner,const RunQueueBuildInfo& bi)
{
  return makeRef(new RunQueue(runner,bi));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*!
 * \brief Créé une référence sur file avec la politique d'exécution par défaut de \a runner.
 *
 * Si la file est temporaire, il est préférable d'utiliser makeQueue() à la place
 * pour éviter une allocation inutile.
 */
inline Ref<RunQueue>
makeQueueRef(Runner* runner)
{
  ARCANE_CHECK_POINTER(runner);
  return makeRef(new RunQueue(*runner));
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane::Accelerator

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif  
