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
#define SIMTIME get_sim_time()

//structures defined here:
struct msg buf[BUFLEN];
struct pkt resend_buf_A[BUFLEN];
struct pkt resend_buf_B[BUFLEN];

//variables defined here:
int sequenceNum = 0, expectedNum = 0, base = 0;
int buf_head = 0, buf_tail = 0;
int ack_buf[BUFLEN];
int resend_buf_B_status[BUFLEN];
float timer_buf[BUFLEN];
float time_used;

//functions defined here:
int getChecksum(struct pkt p)
{
	int c = 0, i;
	c += p.seqnum;
	c += p.acknum;
	for(i=0; i<20; i+=1)
		c += (int)p.payload[i];
	return c;
}

void sendPacket(int seq ,struct msg message)
{
	struct pkt packet;
	memset(&packet,0,sizeof(packet));
  	strcpy(packet.payload,message.data);
	packet.seqnum = seq;
	packet.checksum = getChecksum(packet);
	tolayer3(0,packet);
	if(seq == base)
		starttimer(0,TIMEOUT);
	
	resend_buf_A[seq] = packet;
	timer_buf[seq] = SIMTIME + TIMEOUT;
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
	int i;
	int checksum = getChecksum(packet);
	int acknum = packet.acknum;
	if(checksum == packet.checksum)
	{
		if((acknum > base) || (acknum <= base+WINDOW))
		{
			timer_buf[acknum] = 0;
			ack_buf[acknum] = 1;
			if(base == acknum)
			{
				for(i=base; i<sequenceNum; i += 1)
				{	
					if(ack_buf[i] != 1)
						break;
					base += 1;
				}
			}
			while((sequenceNum < base+WINDOW) && (buf_head < buf_tail))
			{
				sendPacket(sequenceNum, buf[buf_head]);
				buf_head += 1;		
				sequenceNum += 1;
			}
			if(base == sequenceNum)
				stoptimer(0);
		}
		else
			return;
	}
	else
		printf("Checksum mismatch error");
	return;
}

/* called when A's timer goes off */	
void A_timerinterrupt()
{
	int i;
	for (i= base; i < sequenceNum; i += 1)
	{
		if (timer_buf[i] == SIMTIME)
		{
			if(ack_buf[i] == 0)
			{
				tolayer3(0, resend_buf_A[i]);
				timer_buf[i] = SIMTIME+TIMEOUT;		
				break;
			}
		}
	}
	time_used  = SIMTIME+TIMEOUT;
	for (i= base; i < sequenceNum; i += 1)
	{
		if((ack_buf[i] == 0) && (timer_buf[i] < time_used))
			time_used = timer_buf[i];
	}
	if (time_used > 0)
		starttimer(0, time_used - SIMTIME);
	return;
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
	memset(&buf, 0, sizeof(buf));
	memset(&resend_buf_A, 0, sizeof(resend_buf_A));
	memset(&timer_buf, 0, sizeof(timer_buf));
	memset(&ack_buf, 0, sizeof(ack_buf));
	return;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
  struct pkt packet;
{
	int checksum = getChecksum(packet);
	if(checksum == packet.checksum)
	{
		if((packet.seqnum >= expectedNum+WINDOW) || (packet.seqnum < expectedNum-WINDOW))
			return;
		else
		{
			struct pkt ackpkt;
			memset(&ackpkt, 0, sizeof(ackpkt));
			ackpkt.acknum = packet.seqnum;
			ackpkt.checksum = getChecksum(ackpkt);
			tolayer3(1, ackpkt);

			if (packet.seqnum != expectedNum)
			{
				resend_buf_B[packet.seqnum] = packet;
				resend_buf_B_status[packet.seqnum] = 1;
			}
			else
			{	
				tolayer5(1, packet.payload);
				expectedNum += 1;
				for (int i=expectedNum; i < expectedNum+WINDOW; i+=1)
				{
					if (resend_buf_B_status[i]==0)
						break;
					tolayer5(1, resend_buf_B[i].payload);
					expectedNum += 1;
				}
			}
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
	memset(&resend_buf_B , 0, sizeof(resend_buf_B));
	memset(&resend_buf_B_status , 0, sizeof(resend_buf_B_status));
	return;
}
