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

//structures defined here:
struct msg buf[BUFLEN];
struct pkt tempPck;

//variables defined here:
int sequenceNum = 0, A_status, B_status;
int buf_head = 0, buf_tail = 0;

//function defined here:
int getChecksum(struct pkt p)
{
	int c = 0, i;
	c += p.seqnum;
	c += p.acknum;
	for(i=0; i<20; i+=1)
		c += (int)p.payload[i];
	return c;
}

void sendPacket(int seqNum, struct msg message)
{
	struct pkt p;
	memset(&p, 0, sizeof(p));
	p.seqnum = seqNum;
	strcpy(p.payload, message.data);
	p.checksum = getChecksum(p);
	A_status = 1;
	starttimer(0, TIMEOUT);
	tolayer3(0, p);
	tempPck = p;
	return;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(message)
  struct msg message;
{
	//only send for the pack lost and send pkt from buf.
	if (A_status == 1)
	{
		buf[buf_tail] = message;
		buf_tail += 1;
	}
	else
		sendPacket(sequenceNum, message);	
	return;
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(packet)
  struct pkt packet;
{
	int checksum = getChecksum(packet);
	if(checksum == packet.checksum)
	{
		if(sequenceNum == packet.acknum)
		{
			stoptimer(0);
			sequenceNum = sequenceNum == 0 ? 1 : 0;
			if (buf_head < buf_tail)
			{
				sendPacket(sequenceNum, buf[buf_head]);
				buf_head += 1;
			}
			else
				A_status = 0;
		}
		else
			printf ("Sequence mistmatch error! \n");
	}
	else
		printf ("Checksum mistmatch error! \n");
	return;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
	tolayer3(0, tempPck);
	starttimer(0, TIMEOUT);
	return;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	A_status = 0;
	memset(&tempPck, 0, sizeof(tempPck));
	memset(&buf, 0, sizeof(buf));
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{	
	int checksum = getChecksum(packet);
	if(checksum == packet.checksum)
	{
		struct pkt ackpkt;
		memset(&ackpkt, 0, sizeof(ackpkt));
		ackpkt.acknum = packet.seqnum;
		ackpkt.checksum = getChecksum(ackpkt);
		tolayer3(1, ackpkt);

		if (packet.seqnum == B_status)
		{
			tolayer5 (1, packet.payload);
			B_status = B_status == 0 ? 1 : 0;
		}
	}
	else
		printf ("Checksum mismatch error! \n");
	return;
}		
/* the following routine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
	B_status = 0;
}
