/**
 * Author: Dennis Leroy Wigand
 * Date:   10 Jul 2018
 *
 */

#include "Sender.hpp"
#include <rtt/Component.hpp> // needed for the macro at the end of this file

#include <stdio.h>

Sender::Sender(std::string const &name) : cogimon::RTTIntrospectionBase(name), portsArePrepared(false), out_Sender_var(0)
{
}

bool Sender::configureHookInternal()
{
	counter = 0;
	if (!portsArePrepared)
	{
		preparePorts();
	}
	return true;
}

bool Sender::startHookInternal()
{
	startTime = this->getSimulationTime();
	var_exec = 0;
	out_exec.write(var_exec);
	RTT::log(RTT::Debug) << this->getName() << "started" << RTT::endlog();
	return true;
}

void Sender::updateHookInternal()
{
	// var_exec = 0;
	// out_exec.write(var_exec);
	var_exec = 1;
	out_exec.write(var_exec);
	startUpdateTime = getSimulationTime();
	RTT::log(RTT::Debug)
		<< this->getName() << "update start" << RTT::endlog();
	// while ((getSimulationTime() - startUpdateTime) < 0.5)
	// {
	// 	// RTT::log(RTT::Debug) << this->getName() << " 0.3 >= " << (getSimulationTime() - startUpdateTime) << RTT::endlog();
	// }
	// out_Sender_var = getSimulationTime();
	while ((getSimulationTime() - startUpdateTime) < 0.2)
	{
		// RTT::log(RTT::Debug) << this->getName() << " 0.3 >= " << (getSimulationTime() - startUpdateTime) << RTT::endlog();
	}

	counter++;
	if (counter >= 10)
	{
		counter = 0;
	}
	out_Sender_var = counter;
	writePort(out_Sender_port, out_Sender_var);

	RTT::log(RTT::Debug) << this->getName() << "   SEND > " << out_Sender_var << RTT::endlog();

	var_trigger = true;
	out_trigger.write(var_trigger);
	// var_exec = 1;
	// out_exec.write(var_exec);
	var_exec = -1;
	out_exec.write(var_exec);
	RTT::log(RTT::Debug) << this->getName() << "update end" << RTT::endlog();
}

void Sender::stopHookInternal()
{
}

void Sender::cleanupHookInternal()
{
}

void Sender::preparePorts()
{
	out_Sender_port.setName("out_port");
	out_Sender_port.doc("Output port Sender");
	ports()->addPort(out_Sender_port);

	out_Sender_port.setDataSample(out_Sender_var);

	var_exec = -5;
	out_exec.setName("out_exec");
	out_exec.setDataSample(var_exec);
	ports()->addPort(out_exec);

	var_trigger = false;
	out_trigger.setName("out_trigger");
	out_trigger.setDataSample(var_trigger);
	ports()->addPort(out_trigger);

	portsArePrepared = true;
}

double Sender::getSimulationTime()
{
	return 1E-9 * RTT::os::TimeService::ticks2nsecs(
					  RTT::os::TimeService::Instance()->getTicks());
}

// This macro, as you can see, creates the component. Every component should have this!
ORO_LIST_COMPONENT_TYPE(Sender)
