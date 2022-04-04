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

#pragma once

#include <unordered_map>
#include <optional>

#include <alien/utils/Precomp.h>

namespace Alien
{
//! ReverseIndexer based on a HashTable
class ALIEN_EXPORT DoKReverseIndexer
{
 public:
  typedef std::pair<Arccore::Int32, Arccore::Int32> Index;
  typedef Arccore::Integer Offset;

  DoKReverseIndexer()
  : m_map()
  {}
  virtual ~DoKReverseIndexer() = default;

  //! Returns the Index (i,j) corresponding to an offset
  //! \param off
  //! \return Index (i,j)
  std::optional<Index> operator[](Offset off) const;

  //! Registers a offset and its corresponding index
  //! \param off
  //! \param i
  void record(Offset off, Index i);

  Int32 size() const { return static_cast<Int32>(m_map.size()); }

 private:
  typedef std::unordered_map<Offset, Index> HashTable;
  HashTable m_map;
};

} // namespace Alien
