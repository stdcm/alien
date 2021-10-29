/*
 * Copyright 2020 IFPEN-CEA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "matrix.h"
#include "vector.h"

#include <boost/timer.hpp>

//#include <petscksp.h>

#include <arccore/message_passing_mpi/MpiMessagePassingMng.h>

#include <alien/expression/solver/SolverStater.h>
#include <alien/core/backend/LinearSolverT.h>

#include <alien/ginkgo/backend.h>
#include <alien/ginkgo/options.h>
#include <alien/ginkgo/export.h>

#include <core/solver/cg.hpp>
#include <core/stop/iteration.hpp>
#include <core/stop/residual_norm.hpp>

#include <memory>

namespace Alien
{
// Compile GinkgoLinearSolver.
template class ALIEN_GINKGO_EXPORT LinearSolver<BackEnd::tag::ginkgo>;

} // namespace Alien

namespace Alien::Ginkgo
{
class InternalLinearSolver
: public IInternalLinearSolver<Matrix, Vector>
, public ObjectWithTrace
{
 public:
  typedef SolverStatus Status;

  InternalLinearSolver();

  explicit InternalLinearSolver(const Options& options);

  ~InternalLinearSolver() override = default;

 public:
  // Nothing to do
  void updateParallelMng(
  Arccore::MessagePassing::IMessagePassingMng* pm) override {}

  bool solve(const Matrix& A, const Vector& b, Vector& x) override;

  bool hasParallelSupport() const override { return true; }

  //! Etat du solveur
  const Status& getStatus() const override;

  const SolverStat& getSolverStat() const override { return m_stat; }

  std::shared_ptr<ILinearAlgebra> algebra() const override;

 private:
  Status m_status;

  Arccore::Real m_init_time{ 0.0 };
  Arccore::Real m_total_solve_time{ 0.0 };
  Arccore::Integer m_solve_num{ 0 };
  Arccore::Integer m_total_iter_num{ 0 };

  SolverStat m_stat;
  Options m_options;

 private:
  void checkError(const Arccore::String& msg, int ierr,
                  int skipError = 0) const;
};

InternalLinearSolver::InternalLinearSolver()
{
  boost::timer tinit;
  m_init_time += tinit.elapsed();
}

InternalLinearSolver::InternalLinearSolver(const Options& options)
: m_options(options)
{
  boost::timer tinit;
  m_init_time += tinit.elapsed();
}

void InternalLinearSolver::checkError(const Arccore::String& msg, int ierr, int skipError) const
{
  if (ierr != 0 and (ierr & ~skipError) != 0) {
    alien_fatal([&] {
      cout() << msg << " failed. [code=" << ierr << "]";
      //CHKERRQ(ierr); // warning car macro qui appelle fx qui retourne un int
    });
  }
}

bool InternalLinearSolver::solve(const Matrix& A, const Vector& b, Vector& x)
{
  // Macro "pratique" en attendant de trouver mieux
  boost::timer tsolve;

  int output_level = m_options.verbose() ? 1 : 0;

        // solver's choice
        std::string solver_name = "undefined";
        switch (m_options.solver()) {
            /*case OptionTypes::GMRES:
                solver_name = "gmres";
                break;*/
            case OptionTypes::CG:
                solver_name = "cg";
                break;
            /*case OptionTypes::BiCG:
                solver_name = "bicg";
                break;
            case OptionTypes::BiCGstab:
                solver_name = "bcgs";
                break;*/
            default:
                alien_fatal([&] {
                    cout() << "Undefined solver option";
                });
                break;
        }

        // preconditioner's choice
        std::string precond_name = "undefined";
        switch (m_options.preconditioner()) {
            case OptionTypes::Jacobi:
                precond_name = "jacobi";
                break;
            case OptionTypes::NoPC:
                precond_name = "none";
                break;
            default:
                alien_fatal([&] { cout() << "Undefined Ginkgo preconditioner option"; });
                break;
        }

        std::clog << "--------------------- solve -------------------\n";
        std::clog << "--------------------- numIterationsMax -------------------" << m_options.numIterationsMax() << "\n";
        std::clog << "--------------------- stopCriteriaValue -------------------" << m_options.stopCriteriaValue() << "\n";
        std::clog << "--------------------- precond_name -------------------" << precond_name << "\n";
        std::clog << "--------------------- solver_name -------------------" << solver_name << "\n";


        // Parameter the solver factory
        const double reduction_factor{m_options.stopCriteriaValue()};
        using cg = gko::solver::Cg<double>;
        auto exec = gko::ReferenceExecutor::create();

        auto solver_factory =
          cg::build().with_criteria(
            gko::stop::Iteration::build().with_max_iters(m_options.numIterationsMax()).on(exec),
            gko::stop::ResidualNorm<double>::build().with_reduction_factor(reduction_factor).on(exec))
          .on(exec);

        // Generate the solver
        //auto solver = solver_factory->generate(share(A.internal()));
        // printf("---matrix a pointer : %x\n",A.internal());

        auto p = std::shared_ptr<const gko::matrix::Csr<double,int>>(A.internal(),[](auto * p){ std::cout << " do not delete !" << std::endl;});
        auto solver = solver_factory->generate(share(p));
        solver->apply(lend(b.internal()), lend(x.internal()));

        std::cout << "------------------------ end solver ok ------------ " << std::endl;

       /*
        // get nb iterations + final residual
        checkError("PETSc get iteration number", KSPGetIterationNumber(solver, &m_status.iteration_count));
        checkError("PETSc get residual norm", KSPGetResidualNorm(solver, &m_status.residual));

        // destroy solver + pc
        checkError("PETSc destroy solver context", KSPDestroy(&solver)); // includes a call to PCDestroy


        // update the counters
        ++m_solve_num;
        m_total_iter_num += m_status.iteration_count;
        m_total_solve_time += tsolve.elapsed();

        return m_status.succeeded;*/
  return {};
}

const Alien::SolverStatus&
InternalLinearSolver::getStatus() const
{
  return m_status;
}

ALIEN_GINKGO_EXPORT
std::shared_ptr<ILinearAlgebra>
InternalLinearSolver::algebra() const
{
  return std::make_shared<LinearAlgebra>();
}

ALIEN_GINKGO_EXPORT
IInternalLinearSolver<Matrix, Vector>*
InternalLinearSolverFactory(const Options& options)
{
  return new InternalLinearSolver(options);
}

ALIEN_GINKGO_EXPORT
IInternalLinearSolver<Matrix, Vector>*
InternalLinearSolverFactory()
{
  return new InternalLinearSolver();
}
} // namespace Alien::Ginkgo
