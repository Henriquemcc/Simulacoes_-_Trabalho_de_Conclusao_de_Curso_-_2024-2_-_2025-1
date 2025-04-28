//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "ToyExample/VeinsInetSampleApplication.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

#include "ToyExample/VeinsInetSampleMessage_m.h"

using namespace inet;

Define_Module(VeinsInetSampleApplication);

void VeinsInetSampleApplication::processPacket(std::shared_ptr<inet::Packet> pk)
{
    auto payload = pk->peekAtFront<VeinsInetSampleMessage>();

    EV_INFO << "Received packet: " << payload << endl;

    getParentModule()->getDisplayString().setTagArg("i", 1, "green");

    traciVehicle->changeRoute(payload->getRoadId(), 999.9);
}

/**
 * Verifice se o vetor de sharedPointers contem um elemento.
 * @param vector Vetor de sharedPointers.
 * @param element Elemento procurado.
 */
template <typename T>
bool VeinsInetSampleApplication::sharedPointerVectorContainsElement(const std::vector<std::shared_ptr<T>> vector, T* element)
{
    return std::any_of(vector.begin(), vector.end(), [element](const std::shared_ptr<T>& ptr) {
        return ptr.get() == element;
    });
}

void VeinsInetSampleApplication::socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet)
{
    // Adicionando pacote a lista de pacotes recebidos
    if (!VeinsInetSampleApplication::sharedPointerVectorContainsElement(VeinsInetSampleApplication::receivedMessages, packet))
    {
        VeinsInetSampleApplication::receivedMessages.push_back(std::make_shared<inet::Packet>(*packet));
    }
}

/**
 * Envia todas as mensagens recebidas.
 */
void VeinsInetSampleApplication::sendReceivedMessages()
{
    EV << "Enviando todas as mensagens recebidas" << endl;
    for (const auto& message : VeinsInetSampleApplication::receivedMessages)
    {
        sendPacket(std::make_unique<inet::Packet>(*message));
    }
}

bool VeinsInetSampleApplication::startApplication()
{
    // Enviando a primeira mensagem, por meio de uma simulacao de um acidente
    if (getParentModule()->getIndex() == 0)
    {
        auto callback = [this]()
        {
            getParentModule()->getDisplayString().setTagArg("i", 1, "red");

            traciVehicle->setSpeed(0);

            auto payload = makeShared<VeinsInetSampleMessage>();
            payload->setChunkLength(B(100));
            payload->setRoadId(traciVehicle->getRoadId().c_str());
            timestampPayload(payload);

            auto packet = createPacket("accident");
            packet->insertAtBack(payload);
            sendPacket(std::move(packet));

            // host should continue after 30s
            auto callback = [this]()
            {
                traciVehicle->setSpeed(-1);
            };
            timerManager.create(veins::TimerSpecification(callback).oneshotIn(SimTime(30, SIMTIME_S)));
        };
        timerManager.create(veins::TimerSpecification(callback).oneshotAt(SimTime(20, SIMTIME_S)));

        // Enviando todas as mensagens recebidas
        timerManager.create(veins::TimerSpecification([this](){sendReceivedMessages();}).oneshotAt(SimTime(20, SIMTIME_S)));
    }
    return true;
}

bool VeinsInetSampleApplication::stopApplication()
{

}

VeinsInetSampleApplication::VeinsInetSampleApplication()
{

}

VeinsInetSampleApplication::~VeinsInetSampleApplication()
{

}