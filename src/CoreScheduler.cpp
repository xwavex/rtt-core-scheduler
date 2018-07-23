/**
 * Author: Dennis Leroy Wigand
 * Date:   23 Jul 2018
 *
 */

#include "CoreScheduler.hpp"
#include <rtt/Component.hpp> // needed for the macro at the end of this file

#include <stdio.h>
#include <rtt/extras/SlaveActivity.hpp>

using namespace cosima;

CoreScheduler::CoreScheduler(std::string const &name) : cogimon::RTTIntrospectionBase(name), portsArePrepared(false), in_A_var(false), in_B_var(
																															 false),
										  out_nAB_var(false), treat_as_slaves(true), startTime(0.0), isConfiguredByUser(false)
{
	this->addOperation("treatAsSlaves", &CoreScheduler::treatAsSlaves, this);
	preparePorts();
}

bool CoreScheduler::treatAsSlaves(bool treatAsSlaves)
{
	if (isConfiguredByUser)
	{
		RTT::log(RTT::Error) << "[" << this->getName() << " ] The activity mode can only be changed in a non-configured state of the component." << RTT::endlog();
		return false;
	}
	treat_as_slaves = treatAsSlaves;
	RTT::log(RTT::Warning) << "[" << this->getName() << " ] treatAsSlaves = " << treat_as_slaves << RTT::endlog();
	return true;
}

bool CoreScheduler::configureHookInternal()
{
	tcList.clear();
	std::vector<std::string>
		peerList = this->getPeerList();
	for (auto peerName : peerList)
	{
		RTT::TaskContext *new_block = this->getPeer(peerName);
		if (new_block)
		{
			if (treat_as_slaves)
			{
				RTT::log(RTT::Warning) << this->getName() << " set SLAVE activity for " << peerName << RTT::endlog();
				new_block->setActivity(
					new RTT::extras::SlaveActivity(
						this->getActivity(),
						new_block->engine()));
			}
			tcList.push_back(new_block);
		}
	}
	R = this->getPeer("R");
	if (this->getPeer("S"))
	{
		S = this->getPeer("S");
	}
	else if (this->getPeer("S2"))
	{
		S = this->getPeer("S2");
	}

	isConfiguredByUser = true;
	return true;
}

bool CoreScheduler::startHookInternal()
{
	startTime = this->getSimulationTime();
	var_exec = 0;
	writePort(out_exec, var_exec);
	RTT::log(RTT::Debug) << this->getName() << "started" << RTT::endlog();
	if (S) // && treat_as_slaves ?
	{
		S->start();
	}
	if (R)
	{
		R->start();
	}
	return true;
}

void CoreScheduler::updateHookInternal()
{
	// var_exec = 0;
	// out_exec.write(var_exec);
	var_exec = 1;
	writePort(out_exec, var_exec);
	RTT::log(RTT::Debug) << this->getName() << "update start" << RTT::endlog();

	// out_nAB_port.write(!(in_A_var && in_B_var));
	// for (RTT::TaskContext *tc : tcList)
	// {
	// 	tc->update();
	// }
	if (S)
	{
		if (treat_as_slaves)
		{
			// TODO seems like update() is blocking...!
			S->update();
		}
		else
		{
			// With own Activities and this line below we can have a non-blocking parallel execution
			// S->trigger();
			// S->getActivity()->start();

			// With own Activities and this line below we can have a sequential execution
			// S->engine()->activate();
		}
	}
	if (R)
	{
		if (treat_as_slaves)
		{
			R->update();
		}
		else
		{
			// With own Activities and this line below we can have a non-blocking parallel execution
			// R->trigger();
			// R->getActivity()->start();

			// With own Activities and this line below we can have a sequential execution
		}
	}
	// // In this case it also works if connections are lost!
	// in_A_flow = in_A_port.read(in_A_var);
	// in_B_flow = in_B_port.read(in_B_var);

	// if (in_A_flow == RTT::NoData)
	// {
	// 	in_A_var = false;
	// }

	// if (in_B_flow == RTT::NoData)
	// {
	// 	in_B_var = false;
	// }
	// var_exec = 1;
	// out_exec.write(var_exec);
	var_exec = -1;
	writePort(out_exec, var_exec);
	RTT::log(RTT::Debug) << this->getName() << "update end" << RTT::endlog();
}

void CoreScheduler::stopHookInternal()
{
	if (S)
	{
		S->stop();
	}
	if (R)
	{
		R->stop();
	}
}

void CoreScheduler::cleanupHookInternal()
{
	isConfiguredByUser = false;
}

void CoreScheduler::preparePorts()
{
	// in_A_port.setName("in_A_port");
	// in_A_port.doc("Input port A");
	// ports()->addPort(in_A_port);
	// in_A_flow = RTT::NoData;

	// in_B_port.setName("in_B_port");
	// in_B_port.doc("Input port B");
	// ports()->addPort(in_B_port);
	// in_B_flow = RTT::NoData;

	// out_nAB_var = false;
	// out_nAB_port.setName("out_nAB_port");
	// out_nAB_port.doc("Output port Q");
	// out_nAB_port.setDataSample(!false);
	// ports()->addPort(out_nAB_port);

	var_exec = -5;
	out_exec.setName("out_exec");
	out_exec.setDataSample(var_exec);
	ports()->addPort(out_exec);

	portsArePrepared = true;
}

double CoreScheduler::getSimulationTime()
{
	return 1E-9 * RTT::os::TimeService::ticks2nsecs(
					  RTT::os::TimeService::Instance()->getTicks());
}

//this macro should appear only once per library
ORO_CREATE_COMPONENT_LIBRARY()
// This macro, as you can see, creates the component. Every component should have this!
ORO_LIST_COMPONENT_TYPE(cosima::CoreScheduler)
