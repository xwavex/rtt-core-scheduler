/** 
 * Author: Dennis Leroy Wigand
 * Date:   10 Jul 2018
 *
 */

#pragma once

#include <rtt/Port.hpp>
#include <rtt/TaskContext.hpp>
#include <string>

// header for introspection
#include <rtt-core-extensions/rtt-introspection-base.hpp>

class Receiver : public cogimon::RTTIntrospectionBase
{
public:
  Receiver(std::string const &name);

  bool configureHookInternal();
  bool startHookInternal();
  void updateHookInternal();
  void stopHookInternal();
  void cleanupHookInternal();

  // test
  bool dataOnPortHook(RTT::base::PortInterface *port);

  double getSimulationTime();
  void preparePorts();
  bool portsArePrepared;

private:
  RTT::InputPort<double> in_port;

  RTT::FlowStatus in_flow;

  RTT::OutputPort<int> out_exec;
  int var_exec;

  double in_var;

  double startTime;
  double startUpdateTime;
};
