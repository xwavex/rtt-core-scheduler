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
#include "CoreScheduler.hpp"

using namespace cosima;

CoreSchedulerService::CoreSchedulerService(RTT::TaskContext *owner) : RTT::Service("CoreSchedulerService", owner)
{
    this->gOwner = owner;
    // this->addOperation("newTrajectory", &GazeboCommunication::newTrajectoryModel, this, RTT::OwnThread).doc("Adds trajectory of the specific object.").arg("visual", "Name of the visualobject");
    // this->addOperation("newTrajectoryModel", &GazeboCommunication::newTrajectoryModel, this, RTT::OwnThread).doc("Adds trajectory of the specific model.").arg("model", "Name of the model");
    // this->addOperation("newTrajectoryLink", &GazeboCommunication::newTrajectoryLink, this, RTT::OwnThread).doc("Add tajectory to link of specific model.").arg("model", "Name of the model").arg("link", "Name of the link");

    // this->addOperation("delTrajectory", &GazeboCommunication::delTrajectoryModel, this, RTT::OwnThread).doc("Deletes trajectory of the specific object.").arg("visual", "Name of the visualobject");
    // this->addOperation("delTrajectoryModel", &GazeboCommunication::delTrajectoryModel, this, RTT::OwnThread).doc("Deletes trajectory of the specific model.").arg("model", "Name of the model");
    // this->addOperation("delTrajectoryLink", &GazeboCommunication::delTrajectoryLink, this, RTT::OwnThread).doc("Deletes trajectory of the specific link.").arg("model", "Name of the model").arg("link", "Name of the link");
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

bool CoreSchedulerService::configure()
{
    // Separate the peer task contexts.
    std::vector<RTT::TaskContext *> coreSchedulerPtrs;
    std::vector<RTT::TaskContext *> normalTcPtrs;
    for (auto peerName : gOwner->getPeerList())
    {
        RTT::TaskContext *tc = gOwner->getPeer(peerName);
        if (tc)
        {
            if (CoreScheduler *cs = dynamic_cast<CoreScheduler *>(tc))
            {
                // Core scheduler task context.
                coreSchedulerPtrs.push_back(tc);
            } else {
                // Normal task context.
                normalTcPtrs.push_back(tc);
            }
        }
    }
    // Compare coreSchedulerPtrs with entries in m_execution_order.
    for (auto it = m_execution_order.begin(), e = m_execution_order.end(); it != e;)
    {
        bool found = false;
        for (int j = 0; j < coreSchedulerPtrs.size(); j++)
        {
            if (it->first.compare(coreSchedulerPtrs[j]->getName()) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // CS not found but demanded by order. Remove order from list.
            RTT::log(RTT::Debug) << "Removing " << it->first << "." << RTT::endlog();
            it = m_execution_order.erase(it);
            // TODO throw error
        }
        else
        {
            it++;
        }
    }
    // Remove missing task context from m_ptg_formulas.
    for (auto it = m_ptg_formulas.begin(), e = m_ptg_formulas.end(); it != e;)
    {
        bool found_first = false;
        for (int j = 0; j < normalTcPtrs.size(); j++)
        {
            if (it->first.compare(normalTcPtrs[j]->getName()) == 0)
            {
                found_first = true;
                break;
            }
        }
        bool found_second = false;
        for (int j = 0; j < normalTcPtrs.size(); j++)
        {
            if (it->second.compare(normalTcPtrs[j]->getName()) == 0)
            {
                found_second = true;
                break;
            }
        }
        if (!found_first || !found_second)
        {
            // CS not found but demanded by order. Remove order from list.
            RTT::log(RTT::Debug) << "Removing (" << it->first << "[" << found_first << "], " << it->second << "[" << found_second << "])." << RTT::endlog();
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
                RTT::log(RTT::Error) << "Peer (cs) " << cs << " could not be accessed!" << RTT::endlog();
                return false;
            }
            // get operation createSignalPort
            if (!csPtr->getOperation("createSignalPort"))
            {
                RTT::log(RTT::Error) << "Operation createSignalPort (in cs) " << cs << " could not be accessed!" << RTT::endlog();
                return false;
            }
            RTT::OperationCaller<std::string(std::string)> createSignalPort_meth = csPtr->getOperation("createSignalPort");
            // createSignalPort_meth.ready?
            std::string csSignalPort = createSignalPort_meth(f.first);

            // 2) create barrier condition in potential_cs2 for f.second
            RTT::TaskContext *potential_cs2Ptr = gOwner->getPeer(potential_cs2);
            if (!potential_cs2Ptr)
            {
                RTT::log(RTT::Error) << "Peer (potential_cs2Ptr) " << potential_cs2Ptr << " could not be accessed!" << RTT::endlog();
                return false;
            }
            //registerBarrierCondition(std::string targetTCName, std::vector<std::string> barrierTCNames)
            // get operation registerBarrierCondition
            if (!potential_cs2Ptr->getOperation("registerBarrierCondition"))
            {
                RTT::log(RTT::Error) << "Operation registerBarrierCondition (in potential_cs2) " << potential_cs2 << " could not be accessed!" << RTT::endlog();
                return false;
            }
            RTT::OperationCaller<bool(std::string, std::string)> registerBarrierCondition_meth = potential_cs2Ptr->getOperation("registerBarrierCondition");
            // registerBarrierCondition_meth.ready?
            bool retRegBaCoBa = registerBarrierCondition_meth.call(f.second, f.first);
            // TODO BIG HACK BUT AS LONG AS WE DO NOT CHANGE THE NAMING CONVERNTION THIS WILL WORK!
            std::string genPortName = "ev_port_" + f.first + "_triggers_" + f.second;

            // ############## do below in a separate loop!!!
            // 3) Configure cs and potential_cs2 if not configured already
            if (!csPtr->isConfigured())
            {
                csPtr->configure();
            }
            if (!potential_cs2Ptr->isConfigured())
            {
                potential_cs2Ptr->configure();
            }

            // 4) connect ports
            RTT::log(RTT::Error) << "Connect " << csSignalPort << " to " << genPortName << "." << RTT::endlog();

            RTT::base::PortInterface *csP = csPtr->ports()->getPort(csSignalPort);
            RTT::base::PortInterface *potential_cs2P = potential_cs2Ptr->ports()->getPort(csSignalPort);
            if (!csP || !potential_cs2P)
            {
                RTT::log(RTT::Error) << "Not ports found to connect!" << RTT::endlog();
                return false;
            }
            csP->connectTo(potential_cs2P);
        }
    }
    return true;
}

ORO_SERVICE_NAMED_PLUGIN(cosima::CoreSchedulerService, "CoreSchedulerService")