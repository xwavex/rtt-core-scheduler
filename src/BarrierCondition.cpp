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

#include "BarrierCondition.hpp"

using namespace cosima;

BarrierCondition::BarrierCondition()
{
}

BarrierCondition::BarrierCondition(std::string const &ttcName)
{
    m_targetTaskContextName = ttcName;
}

void BarrierCondition::setBarrierTaskContextNamesBatch(const std::vector<std::string> btcNames)
{
    m_barrierTaskContextNames = btcNames;
}

const std::string BarrierCondition::getTargetTaskContextName()
{
    return m_targetTaskContextName;
}

const std::vector<std::string> BarrierCondition::getBarrierTaskContextNames()
{
    return m_barrierTaskContextNames;
}

void BarrierCondition::addBarrierData(std::shared_ptr<BarrierData> data)
{
    m_barrierData.push_back(data);
}

std::vector<std::shared_ptr<BarrierData>> BarrierCondition::getBarrierData()
{
    return m_barrierData;
}

bool BarrierCondition::isFulfilled()
{
    // TODO
    return false;
}