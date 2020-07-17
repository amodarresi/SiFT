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
 * Authors: Kevin Peters <kevjay@gmail.com>
 *        Hemanth Narra <hemanthnarra222@gmail.com>
 *
 * ResiliNets at The University of Kansas <https://wiki.ittc.ku.edu/resilinets/Main_Page>
 * Minor changes was done by Amir Modarresi
 */

#ifndef SIFT_REPOSITORIES_H
#define SIFT_REPOSITORIES_H

#include <set>
#include <vector>
#include <map>
#include "ns3/event-id.h"

#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"

namespace ns3 {
namespace sift {


/// A Geographic Tuple.
struct GeographicTuple
{
  /// Main address of a neighbor node.
  Ipv4Address nodeAddr;
  Ipv4Address nodeReceivedFrom;

  /// coordinate of the neighbor
  uint32_t xcoord;
  uint32_t ycoord;
  uint32_t zcoord;

  /// velocity of the neighbor in each axis
  int16_t xvelocity;
  int16_t yvelocity;
  int16_t zvelocity;

  Time recordedTime;
  Time dataRecordedTime;
  Time startTime;
  Time expireTime;

  /// this is set when this node has incorrect info and I am trying to update it.
  // this variable helps immensely to cut down on the number of re-updates
  bool isBeingUpdated;

  /// Keep the status of velocity vector. If it is TRUE the value of the
  //  velocity vector should be updated.
  bool isChanged;
};

static inline bool
operator == (const GeographicTuple &a, const GeographicTuple &b)
{
  return (a.nodeAddr == b.nodeAddr);
}

static inline std::ostream&
operator << (std::ostream &os, const GeographicTuple &tuple)
{
  os << "GeographicTuple(nodeAddr=" << tuple.nodeAddr
     << ", nodeReceivedFrom=" << tuple.nodeReceivedFrom
     << ", lastrecordedtime=" << tuple.recordedTime
     << ", dataRecordedTime=" << tuple.dataRecordedTime
     << ", startTime=" << tuple.startTime
     << ", expireTime=" << tuple.expireTime
     << ", isChanged=" << tuple.isChanged
     << ", xlocation=" << tuple.xcoord
     << ", ylocation=" << tuple.ycoord
     << ", zlocation=" << tuple.zcoord
     << ", xvelocity=" << tuple.xvelocity
     << ", yvelocity=" << tuple.yvelocity
     << ", zvelocity=" << tuple.zvelocity << ")";
  return os;
}

typedef std::vector<GeographicTuple> GeographicSet; ///< Geographic Set type.

} // namespace sift
}  // namespace ns3

#endif  /* SIFT_REPOSITORIES_H */
