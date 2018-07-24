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

CoreScheduler::CoreScheduler(std::string const &name) : cogimon::RTTIntrospectionBase(name), m_activeTaskContextIndex(0)
{
	this->addOperation("printDebugInformation", &CoreScheduler::printDebugInformation, this).doc("Prints debug information including registered port, barrier conditions, and so on.");
	this->addOperation("registerBarrierCondition", &CoreScheduler::registerBarrierCondition, this).doc("Registers a new barrier condition for an as peer registered task context.");
	this->addOperation("triggerEventData", &CoreScheduler::triggerEventData, this);
}

bool CoreScheduler::configureHookInternal()
{
	m_tcList.clear();
	std::vector<std::string> peerList = this->getPeerList();
	for (auto peerName : peerList)
	{
		RTT::TaskContext *new_block = this->getPeer(peerName);
		if (new_block)
		{
			RTT::log(RTT::Warning) << "[" << this->getName() << "] set SLAVE activity for " << peerName << RTT::endlog();
			new_block->setActivity(
				new RTT::extras::SlaveActivity(
					this->getActivity(),
					new_block->engine()));
			m_tcList.push_back(new_block);
		}
	}
	return generatePortsAndData();
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
	if (m_tcList.size() <= 0)
	{
		// TODO Error
		RTT::log(RTT::Error) << "[" << this->getName() << "] Error cannot start because I have no peers!" << RTT::endlog();
		return false;
	}
	m_activeTaskContextIndex = 0;
	m_activeTaskContextPtr = m_tcList.at(m_activeTaskContextIndex);
	if (!m_activeTaskContextPtr)
	{
		RTT::log(RTT::Error) << "[" << this->getName() << "] Error cannot start because retrieving the first peer failed!" << RTT::endlog();
		return false;
	}
	m_activeBarrierCondition = m_barrierConditions[m_activeTaskContextPtr->getName()];

	// Start all task contexts
	for (RTT::TaskContext *tc : m_tcList)
	{
		if (tc)
		{
			if (!tc->start())
			{
				RTT::log(RTT::Error) << "[" << this->getName() << "] Error cannot start because tc " + tc->getName() << " won't start." << RTT::endlog();
				// TODO stop all
				return false;
			}
			else
			{
				// DLW debug
				RTT::log(RTT::Warning) << "[" << this->getName() << "] Starting TC " + tc->getName() << RTT::endlog();
			}
		}
		else
		{
			RTT::log(RTT::Error) << "[" << this->getName() << "] Error cannot start because tc is NULL." << RTT::endlog();
			// TODO stop all
			return false;
		}
	}
	return true;
}

void CoreScheduler::updateHookInternal()
{
	// if there is no bc for the tc or it is fulfilled, proceed with the execution.
	// TODO not sure if double checking is really necessary...?
	if ((!m_activeBarrierCondition) || (m_activeBarrierCondition->isFulfilled()))
	{
		m_activeTaskContextPtr->update(); // update() because they are slaves!

		// Prepare for next iteration
		m_activeTaskContextIndex++;
		if (m_activeTaskContextIndex >= m_tcList.size())
		{
			// This is how a new iteration begins!
			m_activeTaskContextIndex = 0;
			// Clear all until now received barrier data. Reset all barrier constraints
			for (auto const &p_bcEntry : m_barrierConditions)
			{
				p_bcEntry.second->resetAllBarrierData();
			}
		}
		m_activeTaskContextPtr = m_tcList.at(m_activeTaskContextIndex);
		m_activeBarrierCondition = m_barrierConditions[m_activeTaskContextPtr->getName()];
	}
	// Do nothing if we haven't fulfilled a barrier condition yet...
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
	return true;
}

bool CoreScheduler::registerBarrierCondition(std::string const &targetTCName, std::string const &barrierTCName)
{
	if (m_barrierConditions.count(targetTCName) > 0)
	{
		m_barrierConditions[targetTCName]->addBarrierTaskContextName(barrierTCName);
	}
	else
	{
		std::shared_ptr<BarrierCondition> bc = std::shared_ptr<BarrierCondition>(new BarrierCondition(targetTCName));
		bc->addBarrierTaskContextName(barrierTCName);
		m_barrierConditions[targetTCName] = bc;
	}
	return true;
}

void CoreScheduler::clearRegisteredBarrierConditions(std::string const &targetTCName)
{
	m_barrierConditions.erase(targetTCName);
}

bool CoreScheduler::dataOnPortHook(RTT::base::PortInterface *port)
{
	RTT::log(RTT::Warning) << "Received Data on " << port->getName() << RTT::endlog();
	std::shared_ptr<BarrierData> data_var = m_mapPortToDataPtr[port];
	if (data_var)
	{
		// TODO yay our data exists, otherwise it would be a great error
		data_var->setDataState(true);

		// check for fulfillment only if data is related to activeBarrierCondition
		bool ret = m_activeBarrierCondition->isBarrierDataRelated(data_var);
		if (m_activeBarrierCondition && ret)
		{
			bool ret2 = m_activeBarrierCondition->isFulfilled();
			if (ret2)
			{
				return true;
			}
		}
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

			std::shared_ptr<RTT::InputPort<bool>> genInputPort = std::shared_ptr<RTT::InputPort<bool>>(new RTT::InputPort<bool>());
			genInputPort->setName(genPortName);
			genInputPort->doc("This port is used to receive external triggers from " + bcEntry + " to " + targetPortName + ".");
			genPortEvInputPtrs.push_back(genInputPort);
			this->ports()->addEventPort(*(genInputPort.get()));
			// associate port with data
			RTT::base::PortInterface *genPortPtr = this->ports()->getPort(genInputPort->getName()); // or direct get interface?
			if (!genPortPtr)
			{
				// TODO Error
				RTT::log(RTT::Error) << "[" + this->getName() + "] genPortPtr not found for " + genPortName << RTT::endlog();
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
	std::string activeBC = "";
	if (m_activeBarrierCondition)
	{
		activeBC += "##### Active BC: " + m_activeBarrierCondition->getTargetTaskContextName() + "\n" + "#####    Data = ";
		for (auto bd : m_activeBarrierCondition->getBarrierData())
		{
			activeBC += "{" + bd->getDataName() + " : " + (bd->getDataState() ? "true" : "false") + "} ";
		}
	}
	RTT::log(RTT::Debug) << "\n##### DEBUG INFORMATION of " << this->getName() << " (CoreScheduler) ON  #####"
						 << "\n"
						 << "##### Registered Peers: ?"
						 << "\n"
						 << "##### Barriers:"
						 << strBarriers
						 << "\n"
						 << activeBC
						 << "\n"
						 << "##### DEBUG INFORMATION of "
						 << this->getName() << " (CoreScheduler) OFF #####"
						 << RTT::endlog();
}

void CoreScheduler::triggerEventData(std::string const &portName, bool value)
{
	// find port
	RTT::base::PortInterface *pif = this->ports()->getPort(portName);
	debugPort.setName("debugPort");
	debugPort.connectTo(pif);
	debugPort.write(value);
	RTT::log(RTT::Warning) << "Debug send " << (value ? "true" : "false") << " to " << portName << RTT::endlog();
	debugPort.disconnect();
	// if (pif)
	// {
	// 	RTT::base::OutputPortInterface *senderPort = dynamic_cast<RTT::base::OutputPortInterface *>(pif->antiClone());
	// 	if (senderPort)
	// 	{
	// 		debugPort
	// 			senderPort->connectTo(pif); // or perhaps the other way around?
	// 		senderPort->getDataSource()->getRawPointer()
	// 		senderPort->write(value);
	// 		// RTT::Service *test = senderPort->createPortObject();
	// 		// test->
	// 	}
	// }
}

//this macro should appear only once per library
ORO_CREATE_COMPONENT_LIBRARY()
// This macro, as you can see, creates the component. Every component should have this!
ORO_LIST_COMPONENT_TYPE(cosima::CoreScheduler)
