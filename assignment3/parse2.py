__author__ = 'Sabyasachee'

import math

packetsize = 40

def success(packet):
    if packet['deque'] != [] and packet['receive'] != 0.0:
        return True
    else:
        return False


if __name__ == '__main__':
    fo = open('/Users/Sabyasachee/Documents/data/ns-allinone-3.21/ns-3.21/assignment3.tr')
    packetdict = {}
    firstTtime = 10
    lastTtime = 0
    for datastring in fo:
        if datastring.find('UdpHeader') > -1:
            tokenlist = datastring.split()
            sender = int(tokenlist[2].split('/')[2])
            time = float(tokenlist[1])
            key = (sender, id)
            if key in packetdict.keys():
                packet = packetdict[key]
                packet['counter'] += 1
            else:
                packet = {}
                packet['counter'] = 1
                packet['id'] = id
                packet['sender'] = sender
                packet['deque'] = []
                packet['enque'] = 0.0
                packet['dropped'] = 0.0
                packet['receive'] = 0.0

            if tokenlist[0] == '+':
                packet['enque'] = time
            elif tokenlist[0] == '-':
                packet['deque'].append(time)
                if firstTtime > time:
                    firstTtime = time
                if lastTtime < time:
                    lastTtime = time
            elif tokenlist[0] == 'r':
                packet['receive'] = time
            else:
                packet['dropped'] = time

            packetdict[key] = packet

    #for packet in packetdict:
    #    print packetdict[packet]['counter']
    totalsize = 0
    print lastTtime
    print firstTtime
    for key in packetdict:
        packet = packetdict[key]
        if success(packet):
            totalsize += packetsize
    throughput = totalsize/(lastTtime - firstTtime)
    throughput *= .008
    print "throughput(Kbps) = ", throughput

    totaltime = 0.0
    totaltimesq = 0.0
    counter = 0
    for key in packetdict:
        packet = packetdict[key]
        if success(packet):
            time = packet['receive'] - packet['enque']
            totaltime += time
            totaltimesq += time * time
            counter += 1
    avgdelay = totaltime/counter
    jittersq = totaltimesq/counter - avgdelay * avgdelay
    jitter = math.sqrt(jittersq)
    print "avgdelay(sec) ", avgdelay
    print "jitter(sec) ", jitter

    fo.close()
