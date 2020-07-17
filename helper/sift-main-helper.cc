/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Amir Modarresi
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
 * Author: Amir Modarresi   <amodarresi@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include <iostream>
#include "sift-main-helper.h"
#include "ns3/sift-helper.h"
#include "ns3/sift-routing.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/node.h"


NS_LOG_COMPONENT_DEFINE ("SiftMainHelper");

namespace ns3 {
 

SiftMainHelper::SiftMainHelper ()
  : m_siftHelper (0)
{
  NS_LOG_FUNCTION (this);
}

SiftMainHelper::SiftMainHelper (const SiftMainHelper &o)
{
  NS_LOG_FUNCTION (this);
  m_siftHelper = o.m_siftHelper->Copy ();
}

SiftMainHelper::~SiftMainHelper ()
{
  NS_LOG_FUNCTION (this);
  delete m_siftHelper;
}

SiftMainHelper &
SiftMainHelper::operator = (const SiftMainHelper &o)
{
  if (this == &o)
    {
      return *this;
    }
  m_siftHelper = o.m_siftHelper->Copy ();
  return *this;
}

void
SiftMainHelper::Install (SiftHelper &siftHelper, NodeContainer nodes)
{
  NS_LOG_DEBUG ("Passed node container");
  delete m_siftHelper;
  m_siftHelper = siftHelper.Copy ();
  for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i)
    {
      Install (*i);
      
     }
}

void
SiftMainHelper::Install (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);
  Ptr<ns3::sift::SiftRouting> sift = m_siftHelper->Create (node);
  sift->SetNode (node);
}

void
SiftMainHelper::SetSiftHelper (SiftHelper &siftHelper)
{
  NS_LOG_FUNCTION (this);
  delete m_siftHelper;
  m_siftHelper = siftHelper.Copy ();
}

 
} // namespace ns3
