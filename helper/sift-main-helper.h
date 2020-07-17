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
 * Author: Amir Modarresi   <amodarresi.ku.edu>
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

#ifndef SIFT_MAIN_HELPER_H
#define SIFT_MAIN_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/sift-routing.h"
#include "ns3/sift-helper.h"

namespace ns3 {
/**
 * \brief Helper class that adds Sift routing to nodes.
 */
class SiftMainHelper
{
public:
  /**
   * Create an SiftMainHelper that makes life easier for people who want to install
   * Sift routing to nodes.
   */
  SiftMainHelper ();
  ~SiftMainHelper ();
  /**
   * \brief Construct a SiftMainHelper from another previously initialized instance
   * (Copy Constructor).
   */
  SiftMainHelper (const SiftMainHelper &);
  void Install (SiftHelper &siftHelper, NodeContainer nodes);
  void SetSiftHelper (SiftHelper &siftHelper);

private:
  void Install (Ptr<Node> node);
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  SiftMainHelper &operator = (const SiftMainHelper &o);
  const SiftHelper *m_siftHelper;
};

} // namespace ns3

#endif /* SIFT_MAIN_HELPER_H */
