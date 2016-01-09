__author__ = 'Sabyasachee and Nishkarsh'


class Packet:
    """Class for Packets"""
    packetCount = 0

    def __init__(self):
        """Constructor for Packet Class"""
        self.id = 0
        self.payload = 0
        self.enqueTimeClient = 0.0
        self.dequeTimeClient = 0.0
        self.rxTimeServer = 0.0
        self.enqueTimeServer = 0.0
        self.dequeTimeServer = 0.0
        self.rxTimeClient = 0.0
        self.timesForwardedClient = 0.0
        self.timesForwardedServer = 0.0
        self.isDropped = False
        self.droppedTime = 0.0
        self.droppedNode = 0

    def incrementForwardedClient(self):
        """records number of dequeueing attempts"""
        self.timesForwardedClient += 1

    def incrementForwardedServer(self):
        self.timesForwardedServer += 1

    def display(self):
        """displays packet contents"""
        print "id : %d" % self.id
        print "payload : %d" % self.payload
        print "enqueTimeClient : %f" % self.enqueTimeClient
        print "dequeTimeClient : %f" % self.dequeTimeClient
        print "rxTimeServer : %f" % self.rxTimeServer
        print "enqueTimeServer : %f" % self.enqueTimeServer
        print "dequeTimeServer : %f" % self.dequeTimeServer
        print "rxTimeClient : %f" % self.rxTimeClient
        print "isDropped : %s" % self.isDropped
        print "droppedTime : %f" % self.droppedTime
        print "droppedNode : %d" % self.droppedNode

def parsePayload(data):
    return int(data[6:len(data)-1])

def parseNode(data):
    dataList = data.split("/")
    if dataList[2] == "0":
        return True
    else:
        return False

def func(packet):
    return packet.enqueTimeClient

if __name__ == '__main__':
    fo = open('/Users/Sabyasachee/Documents/data/ns-allinone-3.21/ns-3.21/udp-echo.tr')
    packetList = []
    for dataString in fo:
        if dataString.find("UdpHeader") > -1:
            tokenList = dataString.split()
            packetId = int(tokenList[18])
            if packetId > len(packetList) - 1 or packetList[packetId] == 0:
                packetList.insert(packetId, Packet())
                Packet.packetCount += 1
            setattr(packetList[packetId], "id", packetId)
            setattr(packetList[packetId], "payload", parsePayload(tokenList[38]))
            time = float(tokenList[1])
            isClient = parseNode(tokenList[2]);
            if tokenList[0] == "+":
                if isClient:
                    setattr(packetList[packetId], "enqueTimeClient", time)
                else:
                    setattr(packetList[packetId], "enqueTimeServer", time)
            elif tokenList[0] == "-":
                if isClient:
                    setattr(packetList[packetId], "dequeTimeClient", time)
                    packetList[packetId].incrementForwardedClient()
                else:
                    setattr(packetList[packetId], "dequeTimeServer", time)
                    packetList[packetId].incrementForwardedServer()
            elif tokenList[0] == "r":
                if isClient:
                    setattr(packetList[packetId], "rxTimeClient", time)
                else:
                    setattr(packetList[packetId], "rxTimeServer", time)
            else:
                setattr(packetList[packetId], "isDropped", True)
                setattr(packetList[packetId], "droppedTime", time)
                if isClient:
                    setattr(packetList[packetId], "droppedNode", 0)
                else:
                    setattr(packetList[packetId], "droppedNode", 1)

    packet = min(packetList, key=lambda p: p.dequeTimeClient)
    print "timeFirstTPacket = %f" % getattr(packet, "dequeTimeClient")
    packet = max(packetList, key=lambda p: p.dequeTimeClient)
    print "timeLastTPacket = %f" % getattr(packet, "dequeTimeServer")
    packet = min(packetList, key=lambda p: p.rxTimeServer)
    print "timeFirstRPacket = %f" % getattr(packet, "rxTimeServer")
    packet = max(packetList, key=lambda p: p.rxTimeClient)
    print "timeLastRPacket = %f" % getattr(packet, "rxTimeClient")

    delaySum = 0.0
    packetsDropped = 0
    bytesDropped = 0
    lostPackets = 0
    tBytesClient, tPacketsClient = 0, 0
    tBytesServer, tPacketsServer = 0, 0
    rBytesClient, rPacketsClient = 0, 0
    rBytesServer, rPacketsServer = 0, 0
    maxPacket = max(packetList, key=lambda p: p.enqueTimeClient)
    minPacket = min(packetList, key=lambda p: p.enqueTimeClient)
    tTimeClient = getattr(maxPacket, "enqueTimeClient") - getattr(minPacket, "enqueTimeClient")
    maxPacket = max(packetList, key=lambda p: p.enqueTimeServer)
    minPacket = min(packetList, key=lambda p: p.enqueTimeServer)
    tTimeServer = getattr(maxPacket, "enqueTimeServer") - getattr(minPacket, "enqueTimeServer")
    maxPacket = max(packetList, key=lambda p: p.rxTimeClient)
    minPacket = min(packetList, key=lambda p: p.rxTimeClient)
    rTimeClient = getattr(maxPacket, "rxTimeClient") - getattr(minPacket, "rxTimeClient")
    maxPacket = max(packetList, key=lambda p: p.rxTimeServer)
    minPacket = min(packetList, key=lambda p: p.rxTimeServer)
    rTimeServer = getattr(maxPacket, "rxTimeServer") - getattr(minPacket, "rxTimeServer")

    for packet in packetList:
        enqueTimeClient = getattr(packet, "enqueTimeClient")
        dequeTimeClient = getattr(packet, "dequeTimeClient")
        rxTimeServer = getattr(packet, "rxTimeServer")
        enqueTimeServer = getattr(packet, "enqueTimeServer")
        dequeTimeServer = getattr(packet, "dequeTimeServer")
        rxTimeClient = getattr(packet, "rxTimeClient")
        payload = getattr(packet, "payload")

        if rxTimeServer > 0:
            tPacketsClient += 1
            tBytesClient += payload
            rPacketsServer += 1
            rBytesServer += payload
            delaySum += rxTimeServer - enqueTimeClient

        if rxTimeClient > 0:
            tPacketsServer += 1
            tBytesServer += payload
            rPacketsClient += 1
            rBytesClient += payload
            delaySum += rxTimeClient - enqueTimeServer

        if (dequeTimeClient > 0.0 and rxTimeServer == 0.0) or (dequeTimeServer > 0.0 and rxTimeClient == 0.0):
            lostPackets += 1

        if getattr(packet, "isDropped"):
            packetsDropped += 1
            bytesDropped += payload

    print "delaySum : %f" % delaySum
    print "tPacketsClient : %d , tBytesClient : %d" % (tPacketsClient, tBytesClient)
    print "tPacketsServer : %d , tBytesServer : %d" % (tPacketsServer, tBytesServer)
    print "rPacketsClient : %d , rBytesClient : %d" % (rPacketsClient, rBytesClient)
    print "rPacketsServer : %d , rBytesServer : %d" % (rPacketsServer, rBytesServer)
    print "lostPackets : %d" % lostPackets
    print ""

    for packet in packetList:
        if getattr(packet, "timesForwardedClient") - 1:
            print "Number of times packet %d forwarded at Client: %d" % (getattr(packet, "id"), getattr(packet,
                "timesForwardedClient") - 1)
        if getattr(packet, "timesForwardedServer") - 1:
            print "Number of times packet %d forwarded at Server: %d" % (getattr(packet, "id"), getattr(packet,
                "timesForwardedServer") - 1)

    print ""
    print "packetsDropped : %d" % packetsDropped
    print "bytesDropped : %d" % bytesDropped
    txThroughputClient = tBytesClient/tTimeClient
    txThroughputServer = tBytesServer/tTimeServer
    rxThroughputClient = rBytesClient/rTimeClient
    rxThroughputServer = rBytesServer/rTimeServer
    print "transmitter Throughput Client : %f bytes/second" % txThroughputClient
    print "transmitter Throughput Server : %f bytes/second" % txThroughputServer
    print "receiver Throughput Client : %f bytes/second" % rxThroughputClient
    print "receiver Throughput Server : %f bytes/second" % rxThroughputServer

    fo.close()

