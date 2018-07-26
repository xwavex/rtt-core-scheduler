/* ============================================================
 *
 * This file is a part of CoSiMA (CogIMon) project
 *
 * Copyright (C) 2018 by Dennis Leroy Wigand <dwigand at cor-lab dot uni-bielefeld dot de>
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

#include "CoreSchedulerService.hpp"
#include <rtt/OperationCaller.hpp>

#define PRELOG(X) (RTT::log(RTT::X) << "[" << this->getName() << "] ")

using namespace cosima;

CoreSchedulerService::CoreSchedulerService(RTT::TaskContext *owner) : RTT::Service("CoreSchedulerService", owner)
{
    this->gOwner = owner;
    this->addOperation("setExecutionOrder", &CoreSchedulerService::setExecutionOrder, this, RTT::OwnThread).doc("Set the execution order of the task contexts for a specific core scheduler.").arg("csName", "Name of the core scheduler task context.").arg("tcNames", "Names [] of the task contexts executed by the core scheduler.");
    this->addOperation("addPTGFormula", &CoreSchedulerService::addPTGFormula, this, RTT::OwnThread).doc("Add a PTG formula in the following form: source(before) -> target(after).").arg("sourceTcName", "Name of the task context that blocks targetTcName (needs to be a peer of exactly one core scheduler).").arg("targetTcName", "Name of the task context that is blocked by sourceTcName (needs to be a peer of exactly one core scheduler).");
    this->addOperation("setInvolvedCoreScheduler", &CoreSchedulerService::setInvolvedCoreScheduler, this, RTT::OwnThread).doc("Set the involved core schedulers.").arg("csNames", "Names [] of the core schedulers.");
    this->addOperation("printDebugInformation", &CoreSchedulerService::printDebugInformation, this).doc("Print debug information.");
}

void CoreSchedulerService::setExecutionOrder(std::string const &csName, std::vector<std::string> tcNames)
{
    // TODO check if csName is already in there?
    m_execution_order[csName] = tcNames;
}

void CoreSchedulerService::addPTGFormula(std::string const &sourceTcName, std::string const &targetTcName)
{
    m_ptg_formulas.push_back(std::make_pair(sourceTcName, targetTcName));
}

void CoreSchedulerService::setInvolvedCoreScheduler(std::vector<std::string> csNames)
{
    m_csTaskContexts = csNames;
}

bool CoreSchedulerService::configure()
{
    // Separate the peer task contexts.
    m_coreSchedulerPtrs.clear();
    m_normalTcPtrs.clear();

    for (auto peerName : gOwner->getPeerList())
    {
        RTT::TaskContext *tc = gOwner->getPeer(peerName);
        if (tc)
        {
            //if (CoreScheduler *cs = dynamic_cast<CoreScheduler *>(tc))
            if (std::find(m_csTaskContexts.begin(), m_csTaskContexts.end(), peerName) != m_csTaskContexts.end())
            {
                // Core scheduler task context.
                m_coreSchedulerPtrs.push_back(tc);
            }
            else
            {
                // Normal task context.
                m_normalTcPtrs.push_back(tc);
            }
        }
    }
    // Compare m_coreSchedulerPtrs with entries in m_execution_order.
    for (auto it = m_execution_order.begin(), e = m_execution_order.end(); it != e;)
    {
        bool found = false;
        for (int j = 0; j < m_coreSchedulerPtrs.size(); j++)
        {
            if (it->first.compare(m_coreSchedulerPtrs[j]->getName()) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // CS not found but demanded by order. Remove order from list.
            PRELOG(Debug) << "Removing " << it->first << "." << RTT::endlog();
            it = m_execution_order.erase(it);
            // TODO throw error
        }
        else
        {
            it++;
        }
    }
    // Remove missing task context from m_ptg_formulas.
    portsToBeConnected.clear();

    for (auto it = m_ptg_formulas.begin(), e = m_ptg_formulas.end(); it != e;)
    {
        bool found_first = false;
        for (int j = 0; j < m_normalTcPtrs.size(); j++)
        {
            if (it->first.compare(m_normalTcPtrs[j]->getName()) == 0)
            {
                found_first = true;
                break;
            }
        }
        bool found_second = false;
        for (int j = 0; j < m_normalTcPtrs.size(); j++)
        {
            if (it->second.compare(m_normalTcPtrs[j]->getName()) == 0)
            {
                found_second = true;
                break;
            }
        }
        if (!found_first || !found_second)
        {
            // CS not found but demanded by order. Remove order from list.
            PRELOG(Debug) << "Removing (" << it->first << "[" << found_first << "], " << it->second << "[" << found_second << "])." << RTT::endlog();
            it = m_ptg_formulas.erase(it);
            // TODO throw error
        }
        else
        {
            it++;
        }
    }
    // iterate over filtered PTG formulas
    for (auto const &f : m_ptg_formulas)
    {
        // f = std::pair("A", "C")
        // check if intern or extern
        std::string cs = "";
        std::string potential_cs2 = "";
        for (auto eo : m_execution_order)
        {
            // eo = std::map("CS1", ["A", "B"]) or std::map("CS2", ["C"])
            // find eo for f.first and f.second
            for (std::string includedTcName : eo.second)
            {
                // TODO check if duple or already set etc...
                if (f.first.compare(includedTcName) == 0)
                {
                    cs = eo.first;
                }
                if (f.second.compare(includedTcName) == 0)
                {
                    potential_cs2 = eo.first;
                }
            }
        }
        // if cs == potential_cs2 -> internal
        // if cs != potential_cs2 -> external
        if (cs.compare(potential_cs2) != 0)
        {
            // 1) create signal port in cs for f.first
            RTT::TaskContext *csPtr = gOwner->getPeer(cs);
            if (!csPtr)
            {
                PRELOG(Error) << "Peer (cs) " << cs << " could not be accessed!" << RTT::endlog();
                return false;
            }
            // get operation createSignalPort
            if (!csPtr->getOperation("createSignalPort"))
            {
                PRELOG(Error) << "Operation createSignalPort (in cs) " << cs << " could not be accessed!" << RTT::endlog();
                return false;
            }
            RTT::OperationCaller<std::string(std::string)> createSignalPort_meth = csPtr->getOperation("createSignalPort");
            // createSignalPort_meth.ready?
            std::string csSignalPort = createSignalPort_meth(f.first);

            // 2) create barrier condition in potential_cs2 for f.second
            RTT::TaskContext *potential_cs2Ptr = gOwner->getPeer(potential_cs2);
            if (!potential_cs2Ptr)
            {
                PRELOG(Error) << "Peer (potential_cs2Ptr) " << potential_cs2Ptr << " could not be accessed!" << RTT::endlog();
                return false;
            }
            //registerBarrierCondition(std::string targetTCName, std::vector<std::string> barrierTCNames)
            // get operation registerBarrierCondition
            if (!potential_cs2Ptr->getOperation("registerBarrierCondition"))
            {
                PRELOG(Error) << "Operation registerBarrierCondition (in potential_cs2) " << potential_cs2 << " could not be accessed!" << RTT::endlog();
                return false;
            }
            RTT::OperationCaller<bool(std::string, std::string)> registerBarrierCondition_meth = potential_cs2Ptr->getOperation("registerBarrierCondition");
            // registerBarrierCondition_meth.ready?
            bool retRegBaCoBa = registerBarrierCondition_meth.call(f.second, f.first);
            // TODO BIG HACK BUT AS LONG AS WE DO NOT CHANGE THE NAMING CONVERNTION THIS WILL WORK!
            std::string genPortName = "ev_port_" + f.first + "_triggers_" + f.second;

            // store in portsToBeConnected
            portsToBeConnected.push_back(std::make_pair(std::make_pair(csPtr, csSignalPort), std::make_pair(potential_cs2Ptr, genPortName)));
        }
    }

    // 3) Configure cs and potential_cs2 if not configured already
    for (int j = 0; j < m_coreSchedulerPtrs.size(); j++)
    {
        if (!m_coreSchedulerPtrs[j]->isConfigured())
        {
            m_coreSchedulerPtrs[j]->configure();
        }
    }

    // 4) connect ports
    for (auto const &entry : portsToBeConnected)
    {
        RTT::TaskContext *sTC = entry.first.first;
        std::string sP = entry.first.second;

        RTT::TaskContext *tTC = entry.second.first;
        std::string tP = entry.second.second;

        if (sTC && tTC)
        {
            PRELOG(Debug) << "Connect " << sTC->getName() << "." << sP << " -> " << tTC->getName() << "." << tP << "." << RTT::endlog();
            RTT::base::PortInterface *sPptr = sTC->ports()->getPort(sP);
            RTT::base::PortInterface *tPptr = tTC->ports()->getPort(tP);
            if (!sPptr || !tPptr)
            {
                PRELOG(Error) << "No port pointers found to connect!" << RTT::endlog();
                return false;
            }
            bool pRet = sPptr->connectTo(tPptr);
            PRELOG(Debug) << "Connected " << sTC->getName() << "." << sP << " -> " << tTC->getName() << "." << tP << " = " << pRet << "." << RTT::endlog();
        }
        else
        {
            PRELOG(Error) << "Connecting ports, cannot find sTC(" << sTC << ") or rTC(" << tTC << ")!" << RTT::endlog();
            return false;
        }
    }
    return true;
}

void CoreSchedulerService::printDebugInformation()
{
    std::string executionOrderInCS = "";
    for (int csPtrIndex = 0; csPtrIndex < m_coreSchedulerPtrs.size(); csPtrIndex++)
    {
        std::string csNameTmp = m_coreSchedulerPtrs[csPtrIndex]->getName();
        if (m_execution_order.count(csNameTmp) > 0)
        {
            executionOrderInCS += "########    Execution Order for  " + csNameTmp + " : {";
            for (auto tc_name : m_execution_order[csNameTmp])
            {
                executionOrderInCS += " " + tc_name + " ";
            }
            executionOrderInCS += "}\n";
        }
    }
    executionOrderInCS += "\n";

    std::string allPortConnectionsBetweenCSs = "";
    bool output_may_not_be_correct = false;
    for (auto const &entry : portsToBeConnected)
    {
        RTT::TaskContext *sTC = entry.first.first;
        std::string sP = entry.first.second;

        RTT::TaskContext *tTC = entry.second.first;
        std::string tP = entry.second.second;

        if (sTC && tTC)
        {
            RTT::base::PortInterface *sPptr = sTC->ports()->getPort(sP);
            RTT::base::PortInterface *tPptr = tTC->ports()->getPort(tP);
            if (!sPptr || !tPptr)
            {
                PRELOG(Warning) << "No port pointers found in portsToBeConnected!" << RTT::endlog();
                output_may_not_be_correct = true;
                continue;
            }
            allPortConnectionsBetweenCSs += "#####    Connected " + sTC->getName() + "." + sPptr->getName() + " -> " + tTC->getName() + "." + tPptr->getName() + "\n";
        }
        else
        {
            PRELOG(Warning) << "Cannot find sTC(" << sTC << ") or rTC(" << tTC << ")!" << RTT::endlog();
            output_may_not_be_correct = true;
            continue;
        }
    }

    RTT::log(RTT::Error) << "######## Print Debug Information for " << this->getName() << "(CoreSchedulerService) ########"
                         << "\n"
                         << "######## Execution orders:\n"
                         << executionOrderInCS
                         << "\n"
                         << "######## Connected ports:\n"
                         << allPortConnectionsBetweenCSs
                         << (output_may_not_be_correct ? "\nThis output may not be correct due to some errors that occurred!\n" : "\n")
                         << RTT::endlog();
}

ORO_SERVICE_NAMED_PLUGIN(cosima::CoreSchedulerService, "CoreSchedulerService")