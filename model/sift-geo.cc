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
 *                  Hemanth Narra <hemanthnarra222@gmail.com>
 * ResiliNets at The University of Kansas <https://wiki.ittc.ku.edu/resilinets/Main_Page>
 *
 * This file is slightly modified by Amir Modarresi to declares and define the
 * internal state of a sift node
 */

///
/// \brief	Implementation of all functions needed for manipulating the internal
///		state of an Sift node.
///

#include "sift-geo.h"
#include <map>
#include "ns3/log.h"


namespace ns3 {
namespace sift {

NS_LOG_COMPONENT_DEFINE ("SiftGeo");

SiftGeo::SiftGeo()
{
}

GeographicTuple*
SiftGeo::FindGeographicTuple (Ipv4Address const &mainAddr)
{
    NS_LOG_FUNCTION(this << mainAddr);
	for (GeographicSet::iterator it = m_geographicSet.begin ();
       it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == mainAddr)
        {
          return &(*it);
        }
    }
  return NULL;
}

void
SiftGeo::InsertGeographicTuple (GeographicTuple const &tuple, bool &needTriggerUpdate)
{
	NS_LOG_FUNCTION(this << tuple);
	for (GeographicSet::iterator it = m_geographicSet.begin ();
       it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == tuple.nodeAddr)
        {
          // Update it
          /*
           * If there is any change in velocity components then that means this should be updated to other nodes
           * as a triggered update. Else they can accurately calculate the co ordinates based on the last
           * known position.
           */
          NS_LOG_DEBUG ("Existing Values: Node:" << it->nodeAddr << " - Position X:" << it->xcoord << " Y:" << it->ycoord
                                                 << " Z:" << it->zcoord << " Velocity X:" << it->xvelocity
                                                 << " Y:" << it->yvelocity << " Z:" << it->zvelocity);
          if (tuple.xvelocity != it->xvelocity || tuple.yvelocity != it->yvelocity || tuple.zvelocity != it->zvelocity)
            {
              NS_LOG_DEBUG ("Velocity vector is changed. Need to send trigger update");
              it->isChanged = true;
              needTriggerUpdate = true;
            }
          //we should not replace *it with tuple but rather replace the values as the tuple might as well
          //have link information associated with it and if replaced, that info will be lost.
          it->xcoord = tuple.xcoord;
          it->ycoord = tuple.ycoord;
          it->zcoord = tuple.zcoord;
          it->xvelocity = tuple.xvelocity;
          it->yvelocity = tuple.yvelocity;
          it->zvelocity = tuple.zvelocity;
          it->recordedTime = tuple.recordedTime;
          it->dataRecordedTime = tuple.dataRecordedTime;
          it->expireTime = tuple.expireTime;
          it->nodeReceivedFrom = tuple.nodeReceivedFrom;
          it->isBeingUpdated = false;
          return;
        }
    }
  NS_LOG_DEBUG ("Added tuple for " << tuple.nodeAddr);
  m_geographicSet.push_back (tuple);
  // if we come here then there is no tuple present in the table.
  // thus set isChanged to true irrespective of what it is.
  UpdateTupleIsChanged (tuple.nodeAddr, true);
}


void
SiftGeo::EraseGeographicTuple (const GeographicTuple &tuple)
{
  NS_LOG_FUNCTION(this << tuple);
  Time now = Simulator::Now ();
  for (GeographicSet::iterator it = m_geographicSet.begin ();
       it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == tuple.nodeAddr)
        {
          /*
           * Instead of erasing the tuple, I am inserting default values so that we do not loose the link information.
           */
          //m_geographicSet.erase (it);
          it->xcoord = 9999;
          it->ycoord = 9999;
          it->zcoord = 9999;
          it->xvelocity = 256;
          it->yvelocity = 256;
          it->zvelocity = 256;
          it->recordedTime = now;
          it->startTime = Seconds (0);
          it->expireTime = Seconds (0);
          it->dataRecordedTime = Seconds (0);
          it->isBeingUpdated = false;
          it->isChanged = false;
          it->nodeReceivedFrom = Ipv4Address ();
          break;
        }
    }
}


void
SiftGeo::UpdateAllTupleIsChanged (bool isChanged)
{
   NS_LOG_FUNCTION(this);
   for (GeographicSet::iterator it = m_geographicSet.begin (); it != m_geographicSet.end (); it++)
    {
      it->isChanged = isChanged;
    }
}

void
SiftGeo::UpdateTupleIsChanged (const Ipv4Address nodeAddress, bool isChanged)
{
   NS_LOG_FUNCTION(this << nodeAddress);
   for (GeographicSet::iterator it = m_geographicSet.begin (); it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == nodeAddress)
        {
          it->isChanged = isChanged;
        }
    }
}

void
SiftGeo::SetIsBeingUpdated (const Ipv4Address nodeAddress, bool isBeingUpdated)
{
	NS_LOG_FUNCTION(this << nodeAddress);
	for (GeographicSet::iterator it = m_geographicSet.begin (); it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == nodeAddress)
        {
          it->isBeingUpdated = isBeingUpdated;
        }
    }
}

bool
SiftGeo::GetIsBeingUpdated (const Ipv4Address nodeAddress)
{
	NS_LOG_FUNCTION(this << nodeAddress);
	for (GeographicSet::iterator it = m_geographicSet.begin (); it != m_geographicSet.end (); it++)
    {
      if (it->nodeAddr == nodeAddress)
        {
          return it->isBeingUpdated;
        }
    }
  return false;
}


} // namespace sift
} // namespace ns3
