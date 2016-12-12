#include "NetworkBase.h"
#include "NCLDebug.h"

NetworkBase::NetworkBase()
	: m_pNetwork(NULL)
	, m_IncomingKb(0.0f)
	, m_OutgoingKb(0.0f)
	, m_SecondTimer(0.0f)
{

}

NetworkBase::~NetworkBase()
{
	Release();
}

//Initialze network
// - external_port_number MUST be specified to allow peers to connect to this computer - default: use any available port number and disable incoming connections
// - This MUST be called before sending/recieving any packets
bool NetworkBase::Initialize(uint16_t external_port_number, size_t max_peers)
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = external_port_number;

	m_pNetwork = enet_host_create(
		(external_port_number == 0) ? NULL : &address,	//the address at which other peers may connect to this host. If NULL, then no peers may connect to the host.
		max_peers,												//the maximum number of peers that should be allocated for the host.
		1,												//the maximum number of channels allowed; if 0, then this is equivalent to ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT
		0,												//downstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth.
		0);												//upstream bandwidth of the host in bytes/second; if 0, ENet will assume unlimited bandwidth.

	if (m_pNetwork == NULL)
	{
		NCLERROR("Unable to initialise Network Host!");
		return false;
	}

	return true;
}

//Closes all connections/sockets and releases all associated memory
// - Note: This is called automatically in destructor
void NetworkBase::Release()
{
	if (m_pNetwork != NULL)
	{
		enet_host_destroy(m_pNetwork);
		m_pNetwork = NULL;

		m_IncomingKb = 0.0f;
		m_OutgoingKb = 0.0f;
		m_SecondTimer = 0.0f;
	}
}

// Attempt to connect to a peer with a given IP4 Addr:Port No
// - Example usage: BeginConnect(127,0,0,1, 1234) to connect to localhost on port 1234
// - Note: ENetPeer pointer is used to identify the peer and is needed to send/recieve packets to that computer
ENetPeer* NetworkBase::ConnectPeer(uint8_t ip_part1, uint8_t ip_part2, uint8_t ip_part3, uint8_t ip_part4, uint16_t port_number)
{
	if (m_pNetwork != NULL)
	{
		ENetAddress address;
		address.port = port_number;

		//Host IP4 address must be condensed into a 32 bit integer
		address.host = (ip_part4 << 24) | (ip_part3 << 16) | (ip_part2 << 8) | (ip_part1);

		ENetPeer* peer = enet_host_connect(m_pNetwork, &address, 2, 0);
		if (peer == NULL)
		{
			NCLERROR("Unable to connect to peer: %d.%d.%d.%d:%d", ip_part1, ip_part2, ip_part3, ip_part4, port_number);
		}

		return peer;
	}
	else
	{
		NCLERROR("Unable to connect to peer: Network not initialized!");
		return NULL;
	}
}


// Enqueues data to be sent to peer computer over the network.
// - Note: All enqueued packets will automatically be sent the next time 'ServiceNetwork' is called
void NetworkBase::EnqueuePacket(ENetPeer* peer, PacketTransportType transport_type, void* packet_data, size_t data_length)
{
	if (m_pNetwork != NULL)
	{
		if (peer != NULL)
		{
			ENetPacket* packet = enet_packet_create(packet_data, data_length, transport_type);
			enet_peer_send(peer, 0, packet);
		}
		else
		{
			NCLERROR("Unable to enqueue packet: Peer not initialized!");
		}
	}
	else
	{
		NCLERROR("Unable to enqueue packet: Network not initialized!");
	}
}


// Locks thread and waits x milliseconds for a network event to trigger. 
// - Returns true if event recieved or false otherwise
// - Events include:
//			ENET_EVENT_TYPE_CONNECT, returned when a peer is connected and has responded via 'BeginConnect' or an external peer is attempting to connect to us and awaiting our confirmation
//			ENET_EVENT_TYPE_RECEIVE, returned when a packet from a peer has been recieved and is awaiting processing
//			ENET_EVENT_TYPE_DISCONNECT, returned when a peer is disconnected
void NetworkBase::ServiceNetwork(float dt, std::function<void(const ENetEvent&)> callback)
{
	if (m_pNetwork != NULL)
	{
		//Handle all incoming packets & send any packets awaiting dispatch
		ENetEvent event;
		while (enet_host_service(m_pNetwork, &event, 0) > 0)
		{
			callback(event);
		}


		//Update Transmit / Recieve bytes per second
		m_SecondTimer += dt;
		if (m_SecondTimer >= 1.0f)
		{
			m_SecondTimer = 0.0f;

			m_IncomingKb = float(m_pNetwork->totalReceivedData / 128.0); // - 8 bits in a byte and 1024 bits in a KiloBit
			m_OutgoingKb = float(m_pNetwork->totalSentData / 128.0);
			m_pNetwork->totalReceivedData = 0;
			m_pNetwork->totalSentData = 0;
		}
	}
	else
	{
		NCLERROR("Unable to service network: Network not initialized!");
	}
}