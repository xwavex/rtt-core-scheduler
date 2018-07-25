/* ============================================================
 *
 * This file is a part of CoSiMA (CogIMon) project
 *
 * Copyright (C) 2018 by Dennis Leroy Wigand <dwigand@techfak.uni-bielefeld.de>
 *
 * This file may be licensed under the terms of the
 * GNU Lesser General Public License Version 3 (the ``LGPL''),
 * or (at your option) any later version.
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the LGPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the LGPL along with this
 * program. If not, go to http://www.gnu.org/licenses/lgpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The development of this software was supported by:
 *   CoR-Lab, Research Institute for Cognition and Robotics
 *     Bielefeld University
 *
 * ============================================================ */

#pragma once

#include <rtt/Port.hpp>
#include <rtt/TaskContext.hpp>
#include <string>
#include <map>
#include <mutex>

#include <boost/lexical_cast.hpp>

// header for introspection
#include <rtt-core-extensions/rtt-introspection-base.hpp>

#include "BarrierCondition.hpp"
#include "BarrierData.hpp"

namespace cosima
{

class CoreScheduler : public cogimon::RTTIntrospectionBase
{
public:
  /**
   * Constructor.
   */
  CoreScheduler(std::string const &name);

  ///////////////////////////////////////////
  // Internal mirrors of the default Orocos
  // life cycle from the introspection base.
  ///////////////////////////////////////////
  bool configureHookInternal();
  bool startHookInternal();
  void updateHookInternal();
  void stopHookInternal();
  void cleanupHookInternal();
  ///////////////////////////////////////////

  /**
   * Override to intercept event port triggers.
   */
  bool dataOnPortHook(RTT::base::PortInterface *port);

  /**
   * Returns the time provided by the Orocos API in seconds.
   */
  double getOrocosTime();

  /**
   * Register multiple new barrier condition for targetTCName.
   */
  bool registerBarrierConditionBatch(std::string targetTCName, std::vector<std::string> barrierTCNames);

  /**
   * Register a new barrier condition for targetTCName.
   */
  bool registerBarrierCondition(std::string targetTCName, std::string barrierTCName);

  /**
   * Clear all registered barrier conditions for targetTCName.
   */
  void clearRegisteredBarrierConditions(std::string targetTCName);

  /**
   * Generate the ports and shared data based on the registered barrier conditions.
   */
  bool generatePortsAndData();

  /**
   * Debug external trigger mechanism.
   */
  void triggerEventData(std::string portName, bool value);

private:
  // RTT::InputPort<bool> in_A_port;
  // RTT::InputPort<bool> in_B_port;
  // RTT::FlowStatus in_A_flow;
  // RTT::FlowStatus in_B_flow;
  // RTT::OutputPort<int> out_exec;

  double m_startTime; /**< Start time. Details. */

  /**
   * List holding the TaskContexts.
   * TCs are represented by the registered peers.
   */
  std::vector<RTT::TaskContext *> m_tcList;

  ///////////////////
  /// EXAMPLE:
  /// (A ---lo--> B)
  ///    (C -in-> B)
  ///////////////////

  // /**
  //  * Example event input port that acts as trigger to wait for C so B can be executed.
  //  */
  // RTT::InputPort<BarrierData> ev_trigger_C_B;
  // RTT::FlowStatus ev_trigger_C_B_flow;
  // BarrierData ev_trigger_C_B_data;
  // /**
  //  * Example local trigger to wait for A (to finish) so B can be executed.
  //  */
  // BarrierData lo_trigger_A_B_data;

  // TODO how to generate and dynamically manage the internal state?
  // Use a hashmap to encode the name and ~data~: "ev_trigger_C_B_data" : ~ev_trigger_C_B_data~.
  // Get pointer to data from the hashmap and store them in a condition object,
  // so we do not have to do the look up all the time.
  // Then we also can have for each step/state a condition/barrier to wait for.
  // And this we are going to yield in the update hook!
  // Further we need to make sure that the CoreScheduler with the first component of the chain,
  // is triggered by frequency and triggers all the other CoreScheduler data-flow-basedat the beginning
  // of every iteration.
  // Input can then be some kind of cvs/json format to specify the components, order, and their barrier-conditions.

  /**
   * Map access to all registered barrier conditions.
   */
  std::map<std::string, std::shared_ptr<BarrierCondition>> m_barrierConditions;
  // TODO PORTS GLOBAL UEBER BARRIERS GENERIEREN!

  // std::map < std::shared_ptr<RTT::base::InputPortInterface>, std::shared_ptr<bool> >
  // std::shared_ptr<bool> currentState.getExternalVar( std::shared_ptr<RTT::base::InputPortInterface> );
  // void currentState.setExternalVar(std::shared_ptr<RTT::base::InputPortInterface>, true/false);
  // TODO WIE MACHE ICH DAS FUER LOKALE VARS?

  /**
   * With this we encode the state information.
   */
  std::shared_ptr<BarrierCondition> m_activeBarrierCondition;

  /**
   * Holds the index to the active task context in m_tcList.
   */
  int m_activeTaskContextIndex;

  /**
   * Hold a pointer to the active task context.
   */
  RTT::TaskContext *m_activeTaskContextPtr;

  /**
   * This map allows (hopefully) fast access to the data pointer based on the associated port interface.
   */
  std::map<RTT::base::PortInterface *, std::shared_ptr<BarrierData>> m_mapPortToDataPtr;

  /**
   * Prints debug information including registered port, barrier conditions, and so on.
   */
  void printDebugInformation();

  /**
   * Container for generated input ports. Because I had some problems without such an container before.
   * Perhaps there is a smarter solution to this.
   */
  std::vector<std::shared_ptr<RTT::InputPort<bool>>> genPortEvInputPtrs;

  /**
   * Container for generated output ports. Because I had some problems without such an container before.
   * Perhaps there is a smarter solution to this.
   */
  std::map<std::string, std::shared_ptr<RTT::OutputPort<bool>>> genPortOutputSignalPtrs;

  RTT::OutputPort<bool> debugPort;

  std::mutex mutex;
};

} // namespace cosima
