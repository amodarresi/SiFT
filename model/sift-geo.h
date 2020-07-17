/*
 * Copyright (c) 2010 ResiliNets, University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Hemanth Narra <hemanthnarra222@gmail.com>
 *          Kevin Peters <kevjay@gmail.com>
 * ResiliNets at The University of Kansas <https://wiki.ittc.ku.edu/resilinets/Main_Page>
 *
 * This header file is slightly modified by Amir Modarresi to declares and define
 * internal state of a sift node
 */

/// \brief	This header file declares and define the internal state of a sift node.

#ifndef __SIFT_GEO_H__
#define __SIFT_GEO_H__

#include "ns3/timer.h"
#include <vector>
#include <map>

#include "sift-repository.h"

namespace ns3 {
namespace sift {

struct TimeFragments
{
  Time recordedTime;
  uint16_t fragmentNumber;
};

/// This class encapsulates all data structures needed for maintaining internal state of a sift node.
class SiftGeo
{
  //  friend class Sift;

protected:
  GeographicSet m_geographicSet;
  //GeographicSet m_tempGeographicSet;


public:
  SiftGeo ();

  /** param mainAddr Ip Address of node that its geo coordinate should be found
   *  returns the geo location of the node
   */
  GeographicTuple* FindGeographicTuple (const Ipv4Address &mainAddr);
  /** param tuple geo location of the node
   *  updates geo location of the nodes if it has changed and set needTiggerUpdate param
   */
  void InsertGeographicTuple (GeographicTuple const &tuple, bool &needTriggerUpdate);
  /** param tuple geo location of the node
   *  set the default values for the node
   */
  void EraseGeographicTuple (const GeographicTuple &tuple);

 // void UpdateLinkIsChanged (const Ipv4Address nodeAddress, const Ipv4Address neighborAddress, bool isChanged);
 // void UpdateLinkIsChanged (bool isChanged);
  /** param nodeAddress Ip address of the node
   *  Get the status of the node which is being updated
   */
  bool GetIsBeingUpdated (const Ipv4Address nodeAddress);
  /** param nodeAddress Ip address of the node
   *  set the status of the node to be isBeingUpdated
   */
  void SetIsBeingUpdated (const Ipv4Address nodeAddress, bool isBeingUpdated);
  /**
   * set the status of all node to isChanged.
   */
  void UpdateAllTupleIsChanged (bool isChanged);
  /** param nodeAddress of a node whose geo location is changed
   * set the status of the node to isChanged.
   */
  void UpdateTupleIsChanged (const Ipv4Address nodeAddress, bool isChanged);
  //void CopyToTempGeographicTuple (void);

  //set to store the current topology information
  GeographicSet & GetGeographicInfo ()
  {
    return m_geographicSet;
  }

  //Get temporary geo location of the current node
 /* GeographicSet & GetTempGeographicInfo ()
  {
    return m_tempGeographicSet;
  } */

  bool RemoveNodeFromTempSet (Ipv4Address node1Address);

};

} // namespace sift
} // namespace ns3

#endif
