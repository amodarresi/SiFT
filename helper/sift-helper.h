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

#ifndef SIFT_HELPER_H
#define SIFT_HELPER_H

#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/sift-routing.h"
#include "ns3/node.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/icmpv4-l4-protocol.h"

namespace ns3 {
class SiftHelper
{
public:
  /**
   * Create an SiftHelper that makes life easier for people who want to install
   * Sift routing to nodes.
   */
  SiftHelper ();
  ~SiftHelper ();
  /**
   * \brief Construct an SiftHelper from another previously initialized instance
   * (Copy Constructor).
   */
  SiftHelper (const SiftHelper &);
  /**
   * \internal
   * \returns pointer to clone of this SiftHelper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  SiftHelper* Copy (void) const;
  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created L4 protocol
   */
  Ptr<ns3::sift::SiftRouting> Create (Ptr<Node> node) const;
  /**
   * \param value the value of the attribute indicated by name
   * \brief set the attribute value of a particular attribute declared in name parameter
   */
  void Set (std::string name, const AttributeValue &value);
  void SetNodes (NodeContainer &nodes);
private:
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  SiftHelper & operator = (const SiftHelper &o);
  ObjectFactory m_agentFactory;
  NodeContainer m_nodes;
};

} // namespace ns3

#endif // SIFT_HELPER_H
