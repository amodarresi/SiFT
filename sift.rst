SiFT (Simple Forwarding over Trajectory) Routing
--------------------------------------------

SiFT can be considered as a routing/forwarding protocol with very low overhead in computation for using in wireless ad hoc networks

This model was developed by 
`the ResiliNets research group <http://www.ittc.ku.edu/resilinets>`_
at the University of Kansas.  

SiFT Routing Overview
**************************
This model is based on specification written in [1]. There is no standard or RFC for this model currently.

SiFT can be considered as a reactive protocol and operates on a on-demand behavior. This model doesn't keep any routing table and works based on control flooding which is broadcasting in a controlled way. Nodes need to know geolocation of each other and we use sift-geo class to get the proper information from other nodes. Authors of the model have been assumed that these information can be retrieved by GPS or any other similar techniques;however, they haven't specified any particular method.

By knowing geolocation of the source and destination, the source calculates a trajectory to the destination and adds these geographical information to each packet. Therefore, each node that receives these packets can calculate the trajectory. This information plus geolocation of the last sending nodes are added to a header above the IP header. When a node receives a packet, it calculates its distance from the trajectory and the last sending node and set a timer based on these values. The node which has the less value for its timer forwards the packet and other nodes which are in its wireless range and waiting for their timer to timeout stop their timer and consequently the forwarding process. Therefore, nodes which are closer to the trajectory and farther from the last sending nodes forwards the packet. In order to eliminate transmission of those nodes that are far from the trajectory and out of the wireless range of the forwarding node, they stop forwarding process, if their distance from the trajectory are more than a defined threshold. This threshold can be as long as the wireless range of each node. The whole calculation is done by a simple formula which divides the node distance from the trajectory and the last forwarding node. Therefore, mobility of nodes can be considered in each forwarding process and it is not necessary to keep any extra information. In other words, those node which are closer to the trajectory forward the packets. It can be considered broadcasting along the trajectory.

Although pairwise lines can also be used for the trajectory and they are suitable for paths like vehicular networking. It is not implemented

SiFT in ns-3.22
***************
SiFT is implemented in ns-3.19 and tested on ns-3.21 and ns-3.22 as well. We can not find any incompatibility in any of these versions. Moreover, we compare the SiFT performance with other available adhoc routing protocols in ns-3 without any problem. The results are available at [2]

Helper
******
In order to use SiFT the following Helpers have been implemented:

	SiftHelper
	SiftMainHelper

The example scripts inside ``src/sift/examples/`` demonstrate the use of sift based nodes in different scenarios. 
The helper source can be found inside ``src/sift/helper/sift-main-helper.{h,cc}`` and ``src/sift/helper/sift-helper.{h,cc}``


Example
*******
The example can be found in ``src/sift/examples/``:

* siftSample.cc use SiFT as a routing protocol within a traditional MANETs environment.


Validation
**********
Simulation cases similar have been tested and have comparable results.
manet-routing-compare.cc has been used to compare SiFT with three of other routing protocols.

The results are available in a technical paper at [2].

References
**********
[1] Link for main paper <http://home.deib.polimi.it/filippini/papers/2005_ISWCS_Sift.pdf>
[2] Link for SiFT comparison results < http://www.ittc.ku.edu/resilinets/reports/Modarresi-Sterbenz-2015.pdf>

