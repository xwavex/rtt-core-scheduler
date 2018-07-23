/**
 * Author: Dennis Leroy Wigand
 * Date:   23 Jul 2018
 *
 */

#pragma once

#include <rtt/Port.hpp>
#include <rtt/TaskContext.hpp>
#include <string>

#include <boost/lexical_cast.hpp>

// header for introspection
#include <rtt-core-extensions/rtt-introspection-base.hpp>

namespace cosima {

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
  * Returns the time provided by the Orocos API in seconds.
  */
  double getOrocosTime();

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

  /**
   * Example event input port that acts as trigger to wait for C so B can be executed.
   */
  RTT::InputPort<bool> ev_trigger_C_B;
  RTT::FlowStatus ev_trigger_C_B_flow;
  bool ev_trigger_C_B_data;

  /**
   * Example local trigger to wait for A (to finish) so B can be executed.
   */
  bool lo_trigger_A_B_data;

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
};

}
