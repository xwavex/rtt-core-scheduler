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
  CoreScheduler(std::string const &name);

  bool configureHookInternal();
  bool startHookInternal();
  void updateHookInternal();
  void stopHookInternal();
  void cleanupHookInternal();

  double getSimulationTime();
  void preparePorts();
  bool portsArePrepared;

  bool treatAsSlaves(bool treatAsSlaves);

private:
  // RTT::InputPort<bool> in_A_port;
  // RTT::InputPort<bool> in_B_port;

  // RTT::FlowStatus in_A_flow;
  // RTT::FlowStatus in_B_flow;

  RTT::OutputPort<int> out_exec;
  int var_exec;

  bool in_A_var;
  bool in_B_var;

  bool out_nAB_var;

  bool treat_as_slaves;

  double startTime;

  bool isConfiguredByUser;

  std::vector<RTT::TaskContext *> tcList;
  RTT::TaskContext *R;
  RTT::TaskContext *S;
};

}
