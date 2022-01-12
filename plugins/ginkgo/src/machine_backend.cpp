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

#include <alien/ginkgo/machine_backend.h>

namespace Alien::Ginkgo
{

#ifdef ALIEN_PLUGIN_GINKGO_REF
  std::string ginkgo_executor::target_machine = "reference";
#elif ALIEN_PLUGIN_GINKGO_OMP
  std::string ginkgo_executor::target_machine = "omp";
#elif ALIEN_PLUGIN_GINKGO_CUDA
  std::string ginkgo_executor::target_machine = "cuda";
#elif ALIEN_PLUGIN_GINKGO_HIP
  std::string ginkgo_executor::target_machine = "hip";
#elif ALIEN_PLUGIN_GINKGO_DPCPP
  std::string ginkgo_executor::target_machine = "dpcpp";
#else
  std::string ginkgo_executor::target_machine = "default";
#endif


std::map<std::string, std::function<std::shared_ptr<gko::Executor>()>>
ginkgo_executor::exec_map{
  { "omp",
    [] {
        std::cout <<"----------- Using Alien plugin GINKGO with OMP Executor" << std::endl;
        return gko::OmpExecutor::create();
    }
  },
  { "cuda",
    [] {
        std::cout <<"----------- Using Alien plugin GINKGO with CUDA Executor" << std::endl;
        return gko::CudaExecutor::create(0, gko::OmpExecutor::create(), true);
    }
  },
  { "hip",
    [] {
        std::cout <<"----------- Using Alien plugin GINKGO with HIP Executor" << std::endl;
        return gko::HipExecutor::create(0, gko::OmpExecutor::create(), true);
    }
  },
  { "dpcpp",
    [] {
        std::cout <<"----------- Using Alien plugin GINKGO with DPCPP Executor" << std::endl;
        return gko::DpcppExecutor::create(0, gko::OmpExecutor::create());
    }
  },
  { "reference",
      [] {
          std::cout <<"----------- Using Alien plugin GINKGO with REFERENCE Executor" << std::endl;
          return gko::ReferenceExecutor::create();
      }
  },
    { "default",
      [] {
        std::cout <<"----------- No Ginkgo Executor specified !" << std::endl;
        std::cout <<"----------- Using FALLBACK : Alien plugin GINKGO with REFERENCE Executor" << std::endl;
        return gko::ReferenceExecutor::create();
      }
    }
};
} // namespace Alien::Ginkgo
