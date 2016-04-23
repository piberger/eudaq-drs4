/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework - Event Structure
** 
** <VX1742Event>.cpp
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/




#include "VX1742Event.hh"


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

/*

int VX1742Event::setData(header* newhead, const uint32_t* newbuffer, int newbuflen) {
	newbuflen *= sizeof(uint32_t);
	resize_rawbuffer(newbuflen);
	if (buffer == NULL) {
		printf("VX1742Event: could not (re)allocate event data buffer of size %d\n!", newbuflen);
		bufsize=0;
		return -2;
	}

	// copy given data
	memcpy(buffer, newbuffer, bufsize);		// FIXME: check alignmentA
	for(int i=0; i<4; i++){
		head.raw[i] = newhead->raw[i];
	}
	//memcpy(&head, newhead, sizeof(v1742_header));
	return 0;
}

void VX1742Event::invalidate(){
	for(int i=0;i<4;i++){
		head.raw[i]=0;
	}
}

bool VX1742Event::isValid() const {
//	printf("?is Valid Buffer %p\n", buffer);
	if (buffer == NULL) return false;
//	printf("?is Valid A %x\n", head.size.A);
	if (head.size.A != 0xA) return false;
//	printf("?is Valid eventSize %x\n", head.size.eventSize);
	if (head.size.eventSize == 0 ) return false;
	return true;
}

/// retrieve one channels data from the event structure
/// \return number of valid data in array
/// \param[in] chan channel
/// \param[in] array pointer to memory location, where to put channel data
/// \param[in] arraylen maximum number of elements in array
/// \param[out] arraylen number of valid data in array
size_t VX1742Event::getChannelData(unsigned int grp, unsigned int chan, uint16_t* array, size_t arraylen) const{
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	if ( chan >= caen_v1742::CHANNELS_PER_GROUP ) throw llbad_VX1742Event_invalid_channel();
	if ( grp > caen_v1742::GROUPS ) throw llbad_VX1742Event_invalid_group();
	int grppos = getGrpPos(grp);
	if (grppos<0) {
		printf("Group %d NOT FOUND in available channels: 0x%02X\n", grp, this->GroupMask() );
		throw llbad_VX1742Event_disabled_group();
	}

	size_t word_offset= grppos*(this->SamplesPerChannel()*caen_v1742::CHANNELS_PER_GROUP*caen_v1742::RESOLUTION/32);
	
	for (unsigned int i=0; i<this->SamplesPerChannel() && i<arraylen; i++) {
		array[i]=getSample(chan, i, buffer+word_offset);
	}

	return min(arraylen,this->SamplesPerChannel());
}

size_t VX1742Event::getChannelVoltage(unsigned int grp, unsigned int chan, float* array, size_t arraylen, uint16_t dac_offset) const{
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	if ( chan >= caen_v1742::CHANNELS_PER_GROUP ) throw llbad_VX1742Event_invalid_channel();
	if ( grp > caen_v1742::GROUPS ) throw llbad_VX1742Event_invalid_group();
	int grppos = getGrpPos(grp);
	if (grppos<0) {
		printf("Group %d NOT FOUND in available channels: 0x%02X\n", grp, this->GroupMask() );
		throw llbad_VX1742Event_disabled_group();
	}
	
	size_t word_offset= grppos*(this->SamplesPerChannel()*caen_v1742::CHANNELS_PER_GROUP*caen_v1742::RESOLUTION/32);
	
	for (unsigned int i=0; i<this->SamplesPerChannel() && i<arraylen; i++) {
		uint16_t data=getSample(chan, i, buffer+word_offset);
		array[i]=2.*(((float)data)/4096. - 1. + ((float)dac_offset)/65536.);
	}

	return min(arraylen,this->SamplesPerChannel());
}


uint32_t VX1742Event::EventSize() const {			// number of 32-bit words
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.size.eventSize;
}
uint32_t VX1742Event::BoardID() const { 			// source
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.pattern.board_id;
}
uint32_t VX1742Event::Pattern() const {			// LVDS i/o at Triggertime
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.pattern.pattern;
}
uint32_t VX1742Event::GroupMask() const {			// channels in event
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.pattern.group_mask; //channel_mask;
}
uint32_t VX1742Event::EventCounter() const {			// event ID
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.evnt_cnt.event_counter;
}
uint32_t VX1742Event::TriggerTimeTag() const {		// inter-board trigger time reference
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	return head.trigger_time; 
}

uint32_t VX1742Event::Channels() const {
	if ( ! isValid() ) throw llbad_VX1742Event_invalid();	// invalid event
	uint32_t groups = this->GroupMask();
	uint32_t nog=0;

	do { nog+=groups&1; } while ( groups>>=1 );	// count groups

	return nog*8; //8 channels per group
}


size_t VX1742Event::SamplesPerChannel() const {
	uint32_t channels = this->Channels();
	//printf("Channels/buffersize: %d %d\n", channels, bufsize);
	if (channels == 0){return 0;}
	return ((bufsize*8/caen_v1742::RESOLUTION)/this->Channels());
}


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


const VX1742Event::header* VX1742Event::gethead() const {		// header
	return &head;
}

int VX1742Event::resize_rawbuffer(int to_bytesize) {
	// (re)allocate buffer
#ifdef DEBUG
	printf("Allocating new event buffer with %d byte.\n", to_bytesize);
#endif
	if (buffer == NULL) {
		buffer = (uint32_t*)malloc(to_bytesize);
		bufsize=to_bytesize;
	} else {
		buffer = (uint32_t*)realloc(buffer, to_bytesize);
		bufsize=to_bytesize;
		if ( bufsize == 0 ) buffer=NULL;
	}
	return bufsize;
}

int VX1742Event::rawdata_size() const {
	return bufsize;
}

///get position of group in event
///\return position of grp in event (starting at 0), -1 if grp not in event
int VX1742Event::getGrpPos(unsigned int grp) const {
	int res=0;
	for(unsigned int i=0; i<grp;i++){
		if ( (1<<i) & this->GroupMask() ) {
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

*/
