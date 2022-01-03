#include "../include/simulator.h"
#include<stdio.h>
#include<string.h>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SIX ROUTINES *********/
//defines here:
#define BUFLEN 2000
#define TIMEOUT 30.0
#define WINDOW getwinsize()

//structures here:
struct msg buf[BUFLEN];
struct pkt tempBuf[BUFLEN];

//variables here:
int sequenceNum = 0, expectedNum, base = 0;
int buf_head = 0, buf_tail = 0;

//functions here:
int getChecksum(struct pkt p)
{
	int c = 0, i;
	c += p.seqnum;
	c += p.acknum;
	for(i=0; i<20; i+=1)
		c += (int)p.payload[i];
	return c;
}

void sendPacket(int seqNum ,struct msg message)
{
	struct pkt p;
	memset(&p,0,sizeof(p));
	p.seqnum = seqNum;
	strcpy(p.payload,message.data);
	p.checksum = getChecksum(p);
	tolayer3(0, p);
	if (base == seqNum)
		starttimer(0,TIMEOUT);
	tempBuf[seqNum]  = p;
	return;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
	if (sequenceNum >= base+WINDOW)
	{
		//only send for the pack lost and send pkt from buf.
		buf[buf_tail] = message;
		buf_tail += 1;
	}
	else
	{
		sendPacket(sequenceNum, message);	
		sequenceNum += 1;
	}
	return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int checksum = getChecksum(packet);
	if(checksum == packet.checksum)
	{
		base = packet.acknum + 1;
		while ((sequenceNum < base+WINDOW) && (buf_head < buf_tail))
		{
			sendPacket(sequenceNum,buf[buf_head]);
			buf_head += 1;
			sequenceNum += 1;
		}
		base != sequenceNum ? starttimer(0, TIMEOUT) : stoptimer(0);
	}
	else
		printf("Checksum mismatch error");
	return;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	starttimer(0,TIMEOUT);
	for (int i= base; i <sequenceNum; i++)
		tolayer3(0, tempBuf[i]);
	return;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	memset(&buf,0,sizeof(buf));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int checksum = getChecksum(packet);
	if(checksum == packet.checksum)
	{
		if(packet.seqnum == expectedNum)
		{
			//send ack to layer 3:
			struct pkt ackpkt;
			memset(&ackpkt,0,sizeof(ackpkt));
			ackpkt.acknum = expectedNum;
			ackpkt.checksum = getChecksum(ackpkt);
			tolayer3(1,ackpkt);
			//send data to layer 5:
			tolayer5(1,packet.payload);
			expectedNum++;
		}
		else if(expectedNum!=0)
		{
			//send N-ack to layer 3:
			struct pkt ackpkt;
			memset(&ackpkt,0,sizeof(ackpkt));
			ackpkt.acknum = expectedNum-1;
			ackpkt.checksum = getChecksum(ackpkt);
			tolayer3(1,ackpkt);
		}
	}
	else
		printf("Checksum mismatch error! \n");
	return;
}		

/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	expectedNum = 0;
}
