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

CoreScheduler::CoreScheduler(std::string const &name) : cogimon::RTTIntrospectionBase(name)
{
	this->addOperation("printDebugInformation", &CoreScheduler::printDebugInformation, this).doc("Prints debug information including registered port, barrier conditions, and so on.");
}

bool CoreScheduler::configureHookInternal()
{
	// tcList.clear();
	// std::vector<std::string>
	// 	peerList = this->getPeerList();
	// for (auto peerName : peerList)
	// {
	// 	RTT::TaskContext *new_block = this->getPeer(peerName);
	// 	if (new_block)
	// 	{
	// 		if (treat_as_slaves)
	// 		{
	// 			RTT::log(RTT::Warning) << this->getName() << " set SLAVE activity for " << peerName << RTT::endlog();
	// 			new_block->setActivity(
	// 				new RTT::extras::SlaveActivity(
	// 					this->getActivity(),
	// 					new_block->engine()));
	// 		}
	// 		tcList.push_back(new_block);
	// 	}
	// }
	// R = this->getPeer("R");
	// if (this->getPeer("S"))
	// {
	// 	S = this->getPeer("S");
	// }
	// else if (this->getPeer("S2"))
	// {
	// 	S = this->getPeer("S2");
	// }

	// isConfiguredByUser = true;
	return true;
}

bool CoreScheduler::startHookInternal()
{
	// startTime = this->getOrocosTime();
	// var_exec = 0;
	// writePort(out_exec, var_exec);
	// RTT::log(RTT::Debug) << this->getName() << "started" << RTT::endlog();
	// if (S) // && treat_as_slaves ?
	// {
	// 	S->start();
	// }
	// if (R)
	// {
	// 	R->start();
	// }
	return true;
}

void CoreScheduler::updateHookInternal()
{
	// // var_exec = 0;
	// // out_exec.write(var_exec);
	// var_exec = 1;
	// writePort(out_exec, var_exec);
	// RTT::log(RTT::Debug) << this->getName() << "update start" << RTT::endlog();

	// // out_nAB_port.write(!(in_A_var && in_B_var));
	// // for (RTT::TaskContext *tc : tcList)
	// // {
	// // 	tc->update();
	// // }
	// if (S)
	// {
	// 	if (treat_as_slaves)
	// 	{
	// 		// TODO seems like update() is blocking...!
	// 		S->update();
	// 	}
	// 	else
	// 	{
	// 		// With own Activities and this line below we can have a non-blocking parallel execution
	// 		// S->trigger();
	// 		// S->getActivity()->start();

	// 		// With own Activities and this line below we can have a sequential execution
	// 		// S->engine()->activate();
	// 	}
	// }
	// if (R)
	// {
	// 	if (treat_as_slaves)
	// 	{
	// 		R->update();
	// 	}
	// 	else
	// 	{
	// 		// With own Activities and this line below we can have a non-blocking parallel execution
	// 		// R->trigger();
	// 		// R->getActivity()->start();

	// 		// With own Activities and this line below we can have a sequential execution
	// 	}
	// }
	// // // In this case it also works if connections are lost!
	// // in_A_flow = in_A_port.read(in_A_var);
	// // in_B_flow = in_B_port.read(in_B_var);

	// // if (in_A_flow == RTT::NoData)
	// // {
	// // 	in_A_var = false;
	// // }

	// // if (in_B_flow == RTT::NoData)
	// // {
	// // 	in_B_var = false;
	// // }
	// // var_exec = 1;
	// // out_exec.write(var_exec);
	// var_exec = -1;
	// writePort(out_exec, var_exec);
	// RTT::log(RTT::Debug) << this->getName() << "update end" << RTT::endlog();
}

void CoreScheduler::stopHookInternal()
{
	// if (S)
	// {
	// 	S->stop();
	// }
	// if (R)
	// {
	// 	R->stop();
	// }
}

void CoreScheduler::cleanupHookInternal()
{
}

// 	var_exec = -5;
// 	out_exec.setName("out_exec");
// 	out_exec.setDataSample(var_exec);
// 	ports()->addPort(out_exec);

double CoreScheduler::getOrocosTime()
{
	return 1E-9 * RTT::os::TimeService::ticks2nsecs(
					  RTT::os::TimeService::Instance()->getTicks());
}

bool CoreScheduler::registerBarrierConditionBatch(std::string const &targetTCName, const std::vector<std::string> barrierTCNames)
{
	if (m_barrierConditions.count(targetTCName) > 0)
	{
		// targetTCName already contained so we return, because we do not allow multiple barriers for one TC.
		return false;
	}
	std::shared_ptr<BarrierCondition> bc = std::shared_ptr<BarrierCondition>(new BarrierCondition(targetTCName));
	bc->setBarrierTaskContextNamesBatch(barrierTCNames);
	m_barrierConditions[targetTCName] = bc;
}

bool CoreScheduler::dataOnPortHook(RTT::base::PortInterface *port)
{
	// HIER KOMMEN SPEZIFISCHE PORT CALLS AN: port_A_B ... port_C_B ...!
	std::shared_ptr<BarrierData> data_var = m_mapPortToDataPtr[port];
	if (data_var)
	{
		// TODO yay our data exists, otherwise it would be a great error
		// TODO MUTEX
		data_var->setDataState(true);
	}
	return false; // Like this, we are going to ignore the input and do not trigger execution!
}

bool CoreScheduler::generatePortsAndData()
{
	for (auto const &p_bcEntry : m_barrierConditions)
	{
		std::string targetPortName = p_bcEntry.first;
		std::shared_ptr<BarrierCondition> barrierCondition = p_bcEntry.second;
		for (auto const &bcEntry : barrierCondition->getBarrierTaskContextNames())
		{
			// generate port
			std::string genPortName = "ev_port_" + bcEntry + "_triggers_" + targetPortName;
			RTT::InputPort<bool> genInputPort;
			genInputPort.setName(genPortName);
			genInputPort.doc("This port is used to receive external triggers from " + bcEntry + " to " + targetPortName + ".");
			this->ports()->addEventPort(genInputPort);
			// associate port with data
			RTT::base::PortInterface *genPortPtr = this->ports()->getPort(genInputPort.getName());
			if (!genPortPtr)
			{
				// TODO Error
				return false;
			}
			// TODO should we check if we already have the same pointer registered? that would ne an error then...
			std::shared_ptr<BarrierData> genData = std::shared_ptr<BarrierData>(new BarrierData(genPortName + "_data", false));
			m_mapPortToDataPtr[genPortPtr] = genData; // std::make_shared<bool>(false);
			// add the data now also to the barrier
			barrierCondition->addBarrierData(genData);
		}
	}
	return true;
}

void CoreScheduler::printDebugInformation()
{
	std::string strBarriers = "";
	for (auto const &p_bcEntry : m_barrierConditions)
	{
		std::string targetPortName = p_bcEntry.first;
		std::shared_ptr<BarrierCondition> barrierCondition = p_bcEntry.second;
		for (auto const &bcEntry : barrierCondition->getBarrierTaskContextNames())
		{
			std::string genPortName = "ev_port_" + bcEntry + "_triggers_" + targetPortName;
			RTT::base::PortInterface *genPortPtr = this->ports()->getPort(genPortName);

			std::shared_ptr<BarrierData> genDataPtr = m_mapPortToDataPtr[genPortPtr];
			if (genPortPtr)
			{
				if (genDataPtr)
				{
					strBarriers += "\n#####    " + bcEntry + " -> " + targetPortName + " ( " + genPortPtr->getName() + ", " + genDataPtr->getDataName() + " )";
				}
				else
				{
					strBarriers += "\n#####    " + bcEntry + " -> " + targetPortName + " ( " + genPortPtr->getName() + ", PROBLEM NO DATA )";
				}
			}
			else
			{
				if (genDataPtr)
				{
					strBarriers += "\n#####    " + bcEntry + " -> " + targetPortName + " ( PROBLEM NO PORT, " + genDataPtr->getDataName() + " )";
				}
				else
				{
					strBarriers += "\n#####    " + bcEntry + " -> " + targetPortName + " ( PROBLEM NO PORT )";
				}
			}
		}
	}
	RTT::log(RTT::Debug) << "##### DEBUG INFORMATION of " << this->getName() << " (CoreScheduler) ON  #####"
						 << "\n"
						 << "##### Registered Peers: ?"
						 << "\n"
						 << "##### Barriers:"
						 << strBarriers
						 << "\n"
						 << "##### DEBUG INFORMATION of " << this->getName() << " (CoreScheduler) OFF #####"
						 << RTT::endlog();
}

//this macro should appear only once per library
ORO_CREATE_COMPONENT_LIBRARY()
// This macro, as you can see, creates the component. Every component should have this!
ORO_LIST_COMPONENT_TYPE(cosima::CoreScheduler)
