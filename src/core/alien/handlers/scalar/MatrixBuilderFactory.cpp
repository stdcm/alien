/*
 * Copyright 2023 IFPEN-CEA
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
 *  SPDX-License-Identifier: Apache-2.0
 */

//
// Created by chevalierc on 17/05/23.
//

#include <memory>
#include "IDirectMatrixBuilder.h"

#include <alien/kernels/simple_csr/SimpleCSRDirectMatrixBuilder.h>

namespace Alien::Common
{
ALIEN_EXPORT std::unique_ptr<Alien::Common::IDirectMatrixBuilder> directMatrixBuilderFactory(IMatrix& matrix, DirectMatrixOptions::ResetFlag reset_flag,
                                                                                             DirectMatrixOptions::SymmetricFlag symmetric_flag)
{
  return std::make_unique<SimpleCSRDirectMatrixBuilder>(matrix, reset_flag, symmetric_flag);
}
} // namespace Alien::Common