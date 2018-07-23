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

class Sender : public cogimon::RTTIntrospectionBase
{
public:
  Sender(std::string const &name);

  // bool configureHook();
  // bool startHook();
  // void updateHook();
  // void stopHook();
  // void cleanupHook();

  bool configureHookInternal();
  bool startHookInternal();
  void updateHookInternal();
  void stopHookInternal();
  void cleanupHookInternal();

  double getSimulationTime();
  void preparePorts();
  bool portsArePrepared;

private:
  RTT::OutputPort<double> out_Sender_port;

  double out_Sender_var;

  RTT::OutputPort<int> out_exec;
  int var_exec;

  double startTime;
  double startUpdateTime;
  int counter;
};
