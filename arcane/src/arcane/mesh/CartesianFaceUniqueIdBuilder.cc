﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2021 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* CartesianFaceUniqueIdBuilder.cc                             (C) 2000-2021 */
/*                                                                           */
/* Construction des indentifiants uniques des faces en cartésien.            */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/mesh/DynamicMesh.h"

#include "arcane/core/internal/ICartesianMeshGenerationInfo.h"

#include "arcane/IParallelMng.h"

#include <array>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane::mesh
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Construction des uniqueId() des faces pour un maillage cartésien.
 */
class CartesianFaceUniqueIdBuilder
: public TraceAccessor
{
 public:

  //! Construit une instance pour le maillage \a mesh
  CartesianFaceUniqueIdBuilder(DynamicMesh* mesh);

 public:

  void computeFacesUniqueIdAndOwner();

 private:

  DynamicMesh* m_mesh;
  IParallelMng* m_parallel_mng;
  bool m_is_verbose;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

CartesianFaceUniqueIdBuilder::
CartesianFaceUniqueIdBuilder(DynamicMesh* mesh)
: TraceAccessor(mesh->traceMng())
, m_mesh(mesh)
, m_parallel_mng(mesh->parallelMng())
, m_is_verbose(false)
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Calcule les uniqueId() et les propriétaires.
 *
 * Pour les propriétaires, on considère que toutes les faces appartiennent
 * à ce sous-domaine sauf les faces sur les frontières inférieures qui
 * appartiennent au sous-domaine inférieur dans chaque direction.
 * NOTE: cela n'équilibre pas forcément le nombre de faces fantômes/partagées
 * entre les sous-domaines mais c'est plus facile à calculer.
 */
void CartesianFaceUniqueIdBuilder::
computeFacesUniqueIdAndOwner()
{
  auto* cmgi = ICartesianMeshGenerationInfo::getReference(m_mesh,false);
  if (!cmgi)
    ARCANE_FATAL("No 'ICartesianMeshGenerationInfo'. The mesh is not a cartesian mesh "
                 "or was not generated by 'CartesianMeshGenerator'");

  Integer dimension = m_mesh->dimension();
  if (dimension!=2 && dimension!=3)
    ARCANE_THROW(NotSupportedException,"Bad value '{0}' for dimension. Only dimensions 2 or 3 are supported",
                 dimension);
  Int32 my_rank = m_mesh->parallelMng()->commRank();

  Int64ConstArrayView global_nb_cells = cmgi->globalNbCells();
  info() << "Dim=" << dimension;
  Int64 nb_cell_x = global_nb_cells[0];
  Int64 nb_cell_y = global_nb_cells[1];
  Int64 nb_cell_z = global_nb_cells[2];
  Int64 nb_cell_xy = nb_cell_x * nb_cell_y;

  Int64 nb_face_x = nb_cell_x + 1;
  Int64 nb_face_y = nb_cell_y + 1;
  Int64 nb_face_z = nb_cell_z + 1;
  Int64 nb_face_dir_x = nb_face_x * nb_cell_y;
  Int64 nb_face_dir_y = nb_face_y * nb_cell_x;
  Int64 nb_face_dir_z = nb_cell_x * nb_cell_y;
  Int64 face_offset_z = nb_face_dir_x + nb_face_dir_y;
  Int64 nb_face_xyz = face_offset_z + nb_cell_xy;
  Int64 nb_face_xy = nb_face_dir_x + nb_face_dir_y;
  Int64 total_nb_face_xyz = (nb_face_dir_x + nb_face_dir_y) * nb_cell_z;

  info() << "NB_Cell: X=" << nb_cell_x << " Y=" << nb_cell_y << " Z=" << nb_cell_z
         << " XY=" << nb_cell_xy;
  info() << "NB_Face: X=" << nb_face_x << " Y=" << nb_face_y << " Z=" << nb_face_z
         << " NbDirX=" << nb_face_dir_x << " NbDirY=" << nb_face_dir_y  << " NbDirZ=" << nb_face_dir_z
         << " NbFaceXYZ=" << nb_face_xyz << " OffsetZ=" << face_offset_z
         << " TotalNbFaceXYZ=" << total_nb_face_xyz;

  auto own_cells_offsets = cmgi->ownCellOffsets();
  Int64 own_cell_offset_x = own_cells_offsets[0];
  Int64 own_cell_offset_y = own_cells_offsets[1];
  Int64 own_cell_offset_z = own_cells_offsets[2];
  info() << "CellOffset: X=" << own_cell_offset_x << " Y=" << own_cell_offset_y << " Z=" << own_cell_offset_z;

  auto sub_domain_offsets = cmgi->subDomainOffsets();
  Int64 sub_domain_offset_x = sub_domain_offsets[0];
  Int64 sub_domain_offset_y = sub_domain_offsets[1];
  Int64 sub_domain_offset_z = sub_domain_offsets[2];
  info() << "SubDomainOffset: X=" << sub_domain_offset_x << " Y=" << sub_domain_offset_y << " Z=" << sub_domain_offset_z;

  auto nb_sub_domains = cmgi->nbSubDomains();
  Int32 nb_sub_domain_x = nb_sub_domains[0];
  Int32 nb_sub_domain_y = nb_sub_domains[1];
  Int32 nb_sub_domain_z = nb_sub_domains[2];
  info() << "NbSubDomain: X=" << nb_sub_domain_x << " Y=" << nb_sub_domain_y << " Z=" << nb_sub_domain_z;

  // Détermine le rang inférieur dans chaque direction
  Int32 previous_rank_x = my_rank;
  Int32 previous_rank_y = my_rank;
  Int32 previous_rank_z = my_rank;

  ItemInternalMap& cells_map = m_mesh->cellsMap();
  bool is_verbose = m_is_verbose;

  if (dimension==2){
    if (sub_domain_offset_x>0)
      previous_rank_x = my_rank - 1;
    if (sub_domain_offset_y>0)
      previous_rank_y = my_rank - nb_sub_domain_x;
    info() << "PreviousRank X=" << previous_rank_x << " Y=" << previous_rank_y;
    // Les mailles sont des quadrangles
    std::array<Int64,4> face_uids;
    ENUMERATE_ITEM_INTERNAL_MAP_DATA(iid,cells_map){
      // Récupère l'indice (I,J) de la maille
      Cell cell { iid->value() };
      Int64 uid = cell.uniqueId();
      const Int64 y = uid / nb_cell_x;
      const Int64 x = uid % nb_cell_x;
      if (is_verbose)
        info() << "UID=" << uid << " X=" << x << " Y=" << y
               << " N0=" << cell.node(0).uniqueId()
               << " N1=" << cell.node(1).uniqueId()
               << " N2=" << cell.node(2).uniqueId()
               << " N3=" << cell.node(3).uniqueId();
      // Faces selon Y
      face_uids[0] = (x+0) + ((y+0) * nb_cell_x) + nb_face_dir_x;
      face_uids[2] = (x+0) + ((y+1) * nb_cell_x) + nb_face_dir_x;

      // Faces selon X
      face_uids[1] = (x+1) + (y+0) * nb_face_x;
      face_uids[3] = (x+0) + (y+0) * nb_face_x;

      for( int i=0; i<4; ++i ){
        Face face = cell.face(i);
        if (is_verbose)
          info() << "CELL=" << uid << " Face=" << i << " uid=" << face_uids[i]
                 << " N0=" << face.node(0).uniqueId()
                 << " N1=" << face.node(1).uniqueId();

        face.internal()->setUniqueId(face_uids[i]);
      }
      // Positionne le propriétaire de la face inférieure en X
      if (x==own_cell_offset_x && previous_rank_x!=my_rank)
        cell.face(3).internal()->setOwner(previous_rank_x,my_rank);
      // Positionne le propriétaire de la face inférieure en Y
      if (y==own_cell_offset_y && previous_rank_y!=my_rank)
        cell.face(0).internal()->setOwner(previous_rank_y,my_rank);
    }
  }
  else if (dimension==3){
    if (sub_domain_offset_x>0)
      previous_rank_x = my_rank - 1;
    if (sub_domain_offset_y>0)
      previous_rank_y = my_rank - nb_sub_domain_x;
    if (sub_domain_offset_z>0)
      previous_rank_z = my_rank - (nb_sub_domain_x * nb_sub_domain_y);
    info() << "PreviousRank X=" << previous_rank_x << " Y=" << previous_rank_y << " Z=" << previous_rank_z;
    // Les mailles sont des hexaèdres
    std::array<Int64,6> face_uids;
    ENUMERATE_ITEM_INTERNAL_MAP_DATA(iid,cells_map){
      // Récupère l'indice (I,J) de la maille
      Cell cell { iid->value() };
      Int64 uid = cell.uniqueId();
      Int64 z = uid / nb_cell_xy;
      Int64 v = uid - (z * nb_cell_xy);
      Int64 y = v / nb_cell_x;
      Int64 x = v % nb_cell_x;
      if (is_verbose)
        info() << "UID=" << uid << " X=" << x << " Y=" << y << " Z=" << z
               << " N0=" << cell.node(0).uniqueId()
               << " N1=" << cell.node(1).uniqueId()
               << " N2=" << cell.node(2).uniqueId()
               << " N3=" << cell.node(3).uniqueId()
               << " N4=" << cell.node(4).uniqueId()
               << " N5=" << cell.node(5).uniqueId()
               << " N6=" << cell.node(6).uniqueId()
               << " N7=" << cell.node(7).uniqueId();

      // Faces selon Z
      face_uids[0] = (x+0) + ((y+0) * nb_cell_x) + ((z+0) * nb_face_dir_z) + total_nb_face_xyz;
      face_uids[3] = (x+0) + ((y+0) * nb_cell_x) + ((z+1) * nb_face_dir_z) + total_nb_face_xyz;

      // Faces selon X
      face_uids[1] = (x+0) + ((y+0) * nb_face_x) + ((z+0) * nb_face_xy);
      face_uids[4] = (x+1) + ((y+0) * nb_face_x) + ((z+0) * nb_face_xy);

      // Faces selon Y
      face_uids[2] = (x+0) + ((y+0) * nb_cell_x) + ((z+0) * nb_face_xy) + nb_face_dir_x;
      face_uids[5] = (x+0) + ((y+1) * nb_cell_x) + ((z+0) * nb_face_xy) + nb_face_dir_x;

      for( int i=0; i<6; ++i ){
        Face face = cell.face(i);
        if (is_verbose)
          info() << "CELL=" << uid << " Face=" << i << " uid=" << face_uids[i]
                 << " N0=" << face.node(0).uniqueId()
                 << " N1=" << face.node(1).uniqueId()
                 << " N2=" << face.node(2).uniqueId()
                 << " N3=" << face.node(3).uniqueId();
        face.internal()->setUniqueId(face_uids[i]);
      }

      // Positionne le propriétaire de la face inférieure en X
      if (x==own_cell_offset_x && previous_rank_x!=my_rank)
        cell.face(1).internal()->setOwner(previous_rank_x,my_rank);
      // Positionne le propriétaire de la face inférieure en Y
      if (y==own_cell_offset_y && previous_rank_y!=my_rank)
        cell.face(2).internal()->setOwner(previous_rank_y,my_rank);
      // Positionne le propriétaire de la face inférieure en Z
      if (z==own_cell_offset_z && previous_rank_z!=my_rank)
        cell.face(0).internal()->setOwner(previous_rank_z,my_rank);
    }
  }
  else
    ARCANE_FATAL("Invalid dimension");
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

extern "C++" void
arcaneComputeCartesianFaceUniqueId(DynamicMesh* mesh)
{
  CartesianFaceUniqueIdBuilder f(mesh);
  f.computeFacesUniqueIdAndOwner();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane::mesh

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
