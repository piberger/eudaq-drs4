/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework - Event Structure
** 
** <VX1742Event>.cpp
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

#include "VX1742Event.hh"
#include "VX1742DEFS.hh"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>


VX1742Event::VX1742Event(){
	buffer = NULL;
	bufsize = 0; 
	//bzero(head.raw,16);
	return;	
}

VX1742Event::~VX1742Event(){
	if (buffer != NULL){
		free(buffer); 
		buffer=NULL;}
}

int VX1742Event::setData(header* newhead, const u_int* newbuffer, int newbuflen) {
	newbuflen *= sizeof(u_int);
	resize_rawbuffer(newbuflen);
	if (buffer == NULL) {
		printf("VX1742Event: could not (re)allocate event data buffer of size %d\n!", newbuflen);
		bufsize=0;
		return -1;
	}
	// copy given data
	memcpy(buffer, newbuffer, bufsize); //FIXME: check alignment
	for(int i=0; i<4; i++){
		head.raw[i] = newhead->raw[i];
	}
	return 0;
}

int VX1742Event::resize_rawbuffer(int to_bytesize){
	if (buffer == NULL) {
		buffer = (u_int*)malloc(to_bytesize);
		bufsize=to_bytesize;
	}else{
		buffer = (u_int*)realloc(buffer, to_bytesize);
		bufsize=to_bytesize;
		if ( bufsize == 0 ) buffer=NULL;
	}
	return bufsize;
}

int VX1742Event::rawdata_size() const {
	return bufsize;
}

void VX1742Event::invalidate(){
	for(int i=0;i<4;i++){
		head.raw[i]=0;
	}
}

bool VX1742Event::isValid() const {
	if (buffer == NULL) return false;
	if (head.size.A != 0xA) return false;
	if (head.size.eventSize == 0 ) return false;
	return true;
}

u_int VX1742Event::EventSize() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.size.eventSize;
}
u_int VX1742Event::BoardID() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.pattern.board_id;
}
u_int VX1742Event::Pattern() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.pattern.pattern;
}
u_int VX1742Event::GroupMask() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.pattern.group_mask; //channel_mask;
}
u_int VX1742Event::EventCounter() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.evnt_cnt.event_counter;
}
u_int VX1742Event::TriggerTimeTag() const {
	if (!isValid()) std::printf("Event not valid!");
	return head.trigger_time; 
}

u_int VX1742Event::Channels() const {
	if (!isValid()) std::printf("Event not valid!");
	u_int groups = this->GroupMask();
	u_int temp=0;
	do {temp += groups&1;} while (groups>>=1);
	return temp*8; //8 channels per group
}

size_t VX1742Event::SamplesPerChannel() const {
	u_int channels = this->Channels();
	if (channels == 0){return 0;}
	return ((bufsize*8/VX1742_RESOLUTION)/this->Channels());
}


const VX1742Event::header* VX1742Event::gethead() const{
	return &head;
}

int VX1742Event::getGrpPos(unsigned int grp) const {
	int res=0;
	for(unsigned int i=0; i<grp;i++){
		if ((1<<i) & this->GroupMask()){
			res++;
		}
	}
	if ( (1<<grp) & this->GroupMask() ) {
		return res;
	} else {
		return -1;
	}
}


uint16_t VX1742Event::getSample(unsigned int chan, unsigned int sample, const uint32_t * subbuffer)const{

	uint16_t value;

	//starting bit number relative to start of 9 word cluster of data
	int bit_start=3*12*chan+12*(sample%3);
		
	size_t word_start=9*(sample/3) + bit_start/32;
	bit_start%=32;
		
	//data values crossing word boundaries either start at bit 24 or 28)
	if(bit_start==24) {
		value=(subbuffer[word_start]>>bit_start) & 0xFF;
		value|= (subbuffer[word_start+1] &0xF)<<8;
		
	} else if(bit_start==28){
		value=(subbuffer[word_start]>>bit_start) & 0xF;
		value|= (subbuffer[word_start+1] &0xFF) <<4;
	} else{
		value=(subbuffer[word_start]>>bit_start) & 0xFFF;
	}
	return value;
}


size_t VX1742Event::getChannelData(unsigned int grp, unsigned int chan, uint16_t* array, size_t arraylen) const{
	if (!isValid()) std::printf("Event not valid!");
	if (chan >= VX1742_CHANNELS_PER_GROUP) std::printf("There are only %d channels!\n", VX1742_CHANNELS_PER_GROUP);
	if (grp > VX1742_GROUPS) std::printf("There are only %d groups!\n", VX1742_GROUPS);
	int grppos = getGrpPos(grp);
	if (grppos<0) {
		printf("Group %d NOT FOUND in available channels: 0x%02X\n", grp, this->GroupMask() );
	}

	size_t word_offset= grppos*(this->SamplesPerChannel()*VX1742_CHANNELS_PER_GROUP*VX1742_RESOLUTION/32);
	
	for (unsigned int i=0; i<this->SamplesPerChannel() && i<arraylen; i++) {
		array[i]=getSample(chan, i, buffer+word_offset);
	}

	return std::min(arraylen,this->SamplesPerChannel());
}


size_t VX1742Event::getChannelVoltage(unsigned int grp, unsigned int chan, float* array, size_t arraylen, uint16_t dac_offset) const{
	if (!isValid()) std::printf("Event not valid!");
	if (chan >= VX1742_CHANNELS_PER_GROUP) std::printf("There are only %d channels!\n", VX1742_CHANNELS_PER_GROUP);
	if (grp > VX1742_GROUPS) std::printf("There are only %d groups!\n", VX1742_GROUPS);
	int grppos = getGrpPos(grp);
	if (grppos<0) {
		printf("Group %d NOT FOUND in available channels: 0x%02X\n", grp, this->GroupMask() );
	}
	
	size_t word_offset= grppos*(this->SamplesPerChannel()*VX1742_CHANNELS_PER_GROUP*VX1742_RESOLUTION/32);
	
	for (unsigned int i=0; i<this->SamplesPerChannel() && i<arraylen; i++) {
		uint16_t data=getSample(chan, i, buffer+word_offset);
		array[i]=2.*(((float)data)/4096. - 1. + ((float)dac_offset)/65536.);
	}

	return std::min(arraylen,this->SamplesPerChannel());
}