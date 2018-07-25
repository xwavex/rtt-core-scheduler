/**
 * Author: Dennis Leroy Wigand
 * Date:   23 Jul 2018
 *
 */

#include "CoreScheduler.hpp"
#include <rtt/Component.hpp> // needed for the macro at the end of this file

#include <stdio.h>
#include <rtt/extras/SlaveActivity.hpp>

#define PRELOG(X) (RTT::log(RTT::X) << "[" << this->getName() << "] ")

using namespace cosima;

CoreScheduler::CoreScheduler(std::string const &name) : cogimon::RTTIntrospectionBase(name), m_activeTaskContextIndex(0)
{
	this->addOperation("printDebugInformation", &CoreScheduler::printDebugInformation, this, RTT::ClientThread).doc("Prints debug information including registered port, barrier conditions, and so on.");
	this->addOperation("registerBarrierCondition", &CoreScheduler::registerBarrierCondition, this).doc("Registers a new barrier condition for an as peer registered task context.");
	this->addOperation("triggerEventData", &CoreScheduler::triggerEventData, this, RTT::ClientThread);
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
			PRELOG(Debug) << "Setting slave activity for " << peerName << RTT::endlog();
			new_block->setActivity(
				new RTT::extras::SlaveActivity(
					this->getActivity(),
					new_block->engine()));
			m_tcList.push_back(new_block);
			// Generate output signal port for each tc
			PRELOG(Debug) << "Adding signal port for " << peerName << RTT::endlog();
			std::string genPortName = "signal_port_" + peerName;
			std::shared_ptr<RTT::OutputPort<bool>> genOutputPort = std::shared_ptr<RTT::OutputPort<bool>>(new RTT::OutputPort<bool>());
			genOutputPort->setName(genPortName);
			genOutputPort->doc("This port is used to signal when " + peerName + " finished its execution.");
			genOutputPort->setDataSample(false);
			genPortOutputSignalPtrs[peerName] = genOutputPort;
			this->ports()->addPort(*(genOutputPort.get()));
		}
	}
	return generatePortsAndData();
}

bool CoreScheduler::startHookInternal()
{
	PRELOG(Debug) << "startHook() in progess..." << RTT::endlog();
	if (m_tcList.size() <= 0)
	{
		PRELOG(Error) << "Error cannot start because I have no peers!" << RTT::endlog();
		return false;
	}
	m_activeTaskContextIndex = 0;
	m_activeTaskContextPtr = m_tcList.at(m_activeTaskContextIndex);
	if (!m_activeTaskContextPtr)
	{
		PRELOG(Error) << "Error cannot start because retrieving the first peer failed!" << RTT::endlog();
		return false;
	}
	m_activeBarrierCondition = m_barrierConditions[m_activeTaskContextPtr->getName()];

	// Start all task contexts
	PRELOG(Debug) << "Starting all peer task contexts." << RTT::endlog();
	for (RTT::TaskContext *tc : m_tcList)
	{
		if (tc)
		{
			if (!tc->start())
			{
				PRELOG(Error) << "Error cannot start because tc " + tc->getName() << " won't start!" << RTT::endlog();
				// TODO stop all
				return false;
			}
			else
			{
				PRELOG(Debug) << " Starting tc " + tc->getName() << "." << RTT::endlog();
			}
		}
		else
		{
			PRELOG(Error) << "Error cannot start because tc is NULL!" << RTT::endlog();
			// TODO stop all
			return false;
		}
	}
	PRELOG(Debug) << "startHook() successful." << RTT::endlog();
	return true;
}

void CoreScheduler::updateHookInternal()
{
	PRELOG(Debug) << "updateHook() in progess..." << RTT::endlog();
	// TODO not sure if double checking is really necessary...?
	if ((!m_activeBarrierCondition) || (m_activeBarrierCondition->isFulfilled()))
	{
		PRELOG(Debug) << "Barrier condition fulfilled for " << m_activeTaskContextPtr->getName() << ". Call update()." << RTT::endlog();
		m_activeTaskContextPtr->update();																				   // update() because they are slaves!
		std::shared_ptr<RTT::OutputPort<bool>> signalPort = genPortOutputSignalPtrs.at(m_activeTaskContextPtr->getName()); // assumption here is that the name of a component did not change in the mean time... otherwise I need to use a pointer as key!
		if (signalPort)
		{
			PRELOG(Debug) << "signalling " << m_activeTaskContextPtr->getName() << " done." << RTT::endlog();
			signalPort->write(true);
		}
		else
		{
			// TODO we can also check based on the configuration, that entirely internal tc's do not need to signal to the outside!
			PRELOG(Error) << "No signal port for " << m_activeTaskContextPtr->getName() << "!" << RTT::endlog();
		}
		// Prepare for next iteration
		m_activeTaskContextIndex++;
		if (m_activeTaskContextIndex >= m_tcList.size())
		{
			PRELOG(Debug) << "Prepare for next iteration and reset all received barrier condition data." << RTT::endlog();
			// This is how a new iteration begins.
			m_activeTaskContextIndex = 0;
			// Clear all until now received barrier data. Reset all barrier constraints.
			for (auto &p_bcEntry : m_barrierConditions)
			{
				p_bcEntry.second->resetAllBarrierData();
			}
		}
		m_activeTaskContextPtr = m_tcList.at(m_activeTaskContextIndex);
		if (m_barrierConditions.find(m_activeTaskContextPtr->getName()) == m_barrierConditions.end())
		{
			// Case: We have not found a barrier condition for the execution of the next task context.
			std::lock_guard<std::mutex> lockGuard(mutex); // lock because m_activeBarrierCondition is the only thing that can change here.
			m_activeBarrierCondition = 0;
			// Trigger the thread of the activity to execute its ExecutionEngine and the update() method.
			PRELOG(Debug) << "updateHook() successful. Trigger next iteration, because we do not have a barrier condition for " << m_activeTaskContextPtr->getName() << "." << RTT::endlog();
			this->trigger(); // TODO check for valid execution
		}
		else
		{
			// Case: We have found a barrier condition for the execution of the next task context.
			std::lock_guard<std::mutex> lockGuard(mutex); // lock because m_activeBarrierCondition is the only thing that can change here.
			m_activeBarrierCondition = m_barrierConditions[m_activeTaskContextPtr->getName()];
			PRELOG(Debug) << "Set barrier condition for " << m_activeTaskContextPtr->getName() << " active." << RTT::endlog();
		}
	}
	// Do nothing if we haven't fulfilled a barrier condition yet...
	PRELOG(Debug) << "updateHook() successful. Yield until woken by data event for barrier condition." << RTT::endlog();
}

bool CoreScheduler::dataOnPortHook(RTT::base::PortInterface *port)
{
	PRELOG(Debug) << "dataOnPortHook( " << port->getName() << " ) in progess..." << RTT::endlog();
	std::shared_ptr<BarrierData> data_var = m_mapPortToDataPtr[port];
	if (data_var)
	{
		// TODO yay our data exists, otherwise it would be a great error
		data_var->setDataState(true);
		PRELOG(Debug) << "Set data " << data_var->getDataName() << " to " << data_var->getDataState() << "." << RTT::endlog();
		// check for fulfillment only if data is related to activeBarrierCondition
		std::lock_guard<std::mutex> lockGuard(mutex); // lock because m_activeBarrierCondition is also accessed in updateHook().
		if (m_activeBarrierCondition)
		{
			bool in = m_activeBarrierCondition->isBarrierDataRelated(data_var);
			bool fil = m_activeBarrierCondition->isFulfilled();
			PRELOG(Debug) << "Data is? related: " << in << ", is? fulfilled: " << fil << "." << RTT::endlog();
			if (in && fil)
			{
				PRELOG(Debug) << "dataOnPortHook( " << port->getName() << " ) successful. Notify updateHook()." << RTT::endlog();
				return true;
			}
			else
			{
				PRELOG(Debug) << "Data is not related or fulfilled." << RTT::endlog();
				PRELOG(Debug) << "dataOnPortHook( " << port->getName() << " ) successful. NOT notifying updateHook()." << RTT::endlog();
				return false;
			}
		}
		else
		{
			PRELOG(Debug) << "Data ptr is m_activeBarrierCondition == 0." << RTT::endlog();
			PRELOG(Debug) << "dataOnPortHook( " << port->getName() << " ) successful. NOT notifying updateHook()." << RTT::endlog();
			return false;
		}
	}
	PRELOG(Warning) << "No data associated with port, how can this happen? Did we connect the wrong ports? " << port->getName() << "." << RTT::endlog();
	return false; // Like this, we are going to ignore the input and do not trigger execution!
}

void CoreScheduler::stopHookInternal()
{
	PRELOG(Debug) << "stopHook() in progess..." << RTT::endlog();
	if (m_tcList.size() <= 0)
	{
		PRELOG(Debug) << "No peers to stop." << RTT::endlog();
		return;
	}
	// TODO not sure if we really need this!
	std::lock_guard<std::mutex> lockGuard(mutex); // lock because m_activeBarrierCondition is also accessed in updateHook().
	// stop all task contexts
	PRELOG(Debug) << "Stopping all peer task contexts." << RTT::endlog();
	for (RTT::TaskContext *tc : m_tcList)
	{
		if (tc)
		{
			tc->stop();
		}
		else
		{
			// should never happen!
			PRELOG(Error) << "Error cannot stop peer because tc is NULL!" << RTT::endlog();
		}
	}
	PRELOG(Debug) << "stopHook() successful." << RTT::endlog();
}

void CoreScheduler::cleanupHookInternal()
{
	PRELOG(Debug) << "cleanupHook() in progess..." << RTT::endlog();

	// reset active bc index
	PRELOG(Debug) << "Reset active task context index." << RTT::endlog();
	m_activeTaskContextIndex = 0;

	// reset m_activeTaskContextPtr
	PRELOG(Debug) << "Reset active task context pointer." << RTT::endlog();
	m_activeTaskContextPtr = NULL;

	// reset tc list
	PRELOG(Debug) << "Reset task context list." << RTT::endlog();
	m_tcList.clear();

	// reset m_activeBarrierCondition
	PRELOG(Debug) << "Reset active barrier condition pointer." << RTT::endlog();
	m_activeBarrierCondition.reset();

	// reset m_barrierConditions
	PRELOG(Debug) << "Reset barrier condition map." << RTT::endlog();
	m_barrierConditions.clear();

	// reset ports
	PRELOG(Debug) << "Reset all ports." << RTT::endlog();
	std::vector<std::string> pns = this->ports()->getPortNames();

	for (auto pn : pns)
	{
		auto p = this->getPort(pn);
		if (p)
		{
			p->disconnect();
			this->ports()->removePort(pn);
			PRELOG(Debug) << "Removed port " << pn << RTT::endlog();
		}
	}

	PRELOG(Debug) << "cleanupHook() successful." << RTT::endlog();
}

double CoreScheduler::getOrocosTime()
{
	return 1E-9 * RTT::os::TimeService::ticks2nsecs(
					  RTT::os::TimeService::Instance()->getTicks());
}

bool CoreScheduler::registerBarrierConditionBatch(std::string targetTCName, std::vector<std::string> barrierTCNames)
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

bool CoreScheduler::registerBarrierCondition(std::string targetTCName, std::string barrierTCName)
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

void CoreScheduler::clearRegisteredBarrierConditions(std::string targetTCName)
{
	m_barrierConditions.erase(targetTCName);
}

bool CoreScheduler::generatePortsAndData()
{
	for (auto &p_bcEntry : m_barrierConditions)
	{
		std::string targetPortName = p_bcEntry.first;
		std::shared_ptr<BarrierCondition> barrierCondition = p_bcEntry.second;
		for (auto &bcEntry : barrierCondition->getBarrierTaskContextNames())
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
				PRELOG(Error) << "The generated port pointer was not found for " << genPortName << ". Abort generation!" << RTT::endlog();
				return false;
			}
			// TODO should we check if we already have the same pointer registered? that would ne an error then...
			std::shared_ptr<BarrierData> genData = std::shared_ptr<BarrierData>(new BarrierData(genPortName + "_data", false));

			m_mapPortToDataPtr[genPortPtr] = genData; // std::make_shared<bool>(false);
			// add the data now also to the barrier
			barrierCondition->addBarrierData(genData);
		}
	}
	// TODO debug stuff
	debugPort.setName("debugPort");
	debugPort.setDataSample(false);
	return true;
}

void CoreScheduler::printDebugInformation()
{
	std::string strBarriers = "";
	std::string activeBC = "";
	std::lock_guard<std::mutex> lockGuard(mutex); // lock because m_activeBarrierCondition is also accessed in updateHook().
	{
		for (auto &p_bcEntry : m_barrierConditions)
		{
			std::string targetPortName = p_bcEntry.first;
			std::shared_ptr<BarrierCondition> barrierCondition = p_bcEntry.second;
			for (auto &bcEntry : barrierCondition->getBarrierTaskContextNames())
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
		if (m_activeBarrierCondition)
		{
			activeBC += "##### Active BC: " + m_activeBarrierCondition->getTargetTaskContextName() + "\n" + "#####    Data = ";
			for (auto bd : m_activeBarrierCondition->getBarrierData())
			{
				activeBC += "{" + bd->getDataName() + " : " + (bd->getDataState() ? "true" : "false") + "} ";
			}
		}
	}
	PRELOG(Error) << "\n##### DEBUG INFORMATION of " << this->getName() << " (CoreScheduler) ON  #####"
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

void CoreScheduler::triggerEventData(std::string portName, bool value)
{
	// find port
	RTT::base::PortInterface *pif = this->ports()->getPort(portName);
	debugPort.connectTo(pif);
	debugPort.write(value);
	PRELOG(Warning) << "Debug send " << (value ? "true" : "false") << " to " << portName << "." << RTT::endlog();
	debugPort.disconnect();
}

//this macro should appear only once per library
ORO_CREATE_COMPONENT_LIBRARY()
// This macro, as you can see, creates the component. Every component should have this!
ORO_LIST_COMPONENT_TYPE(cosima::CoreScheduler)
