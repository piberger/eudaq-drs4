/* ---------------------------------------------------------------------------------
** CAEN VX1742 implementation into the EUDAQ framework - Event Structure
** 
** <VX1742Event>.hh
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/


#ifndef VX1742EVENT_HH
#define VX1742EVENT_HH


#include <sys/types.h>
#include <stdint.h>

class VX1742Event{
  
  public:
	union header_size {
	    struct {
	      u_int eventSize		:28;
		  u_int A				:4;
	  };
		u_int raw;
	};


	union header_pattern {
	    struct{
		  u_int group_mask		:4;
		  u_int reserved_1		:4;
		  u_int pattern			:14;
		  u_int reserved_0		:5;
		  u_int board_id		:5;
		};
		u_int raw;
	};

	union header_event_counter {
		struct {
		  u_int event_counter		:22;
		  u_int reserved			:10;
		};
		u_int raw;
	};


	union header {
	  struct {
		union header_size size;
		union header_pattern pattern;
		union header_event_counter evnt_cnt;
		u_int trigger_time;
	  };
	  u_int raw[4];
	};



	//specific event based group stuff:
	union group_header{
	  struct{
	  	u_int groupSize 	   :12;
	  	u_int tr 				:1;
	  	u_int reserved_3		:3;
	  	u_int frequency			:2;
	  	u_int reserved_2		:2;
	  	u_int index_cell	   :10;
	  	u_int reserved_1		:2;
	  };
	  u_int raw;
	};


	union gr_headers{
		struct{
			union group_header grh[4];
		};
		u_int raw[4];
	};


	union group_trailer{
		struct{
			u_int trigger_time		:30;
			u_int reserved 			 :2;
		};
		u_int raw;
	};


	union gr_trailers{
		struct{
			union group_trailer grt[4];
		};
		u_int raw[4];
	};


	VX1742Event();
	virtual ~VX1742Event();
	int setData(header* newhead, const u_int* newbuffer, int newbuflen);
	void invalidate();
	bool isValid() const;
	u_int EventSize() const;	
	u_int BoardID() const;
	u_int Pattern() const;
	u_int GroupMask() const;
	u_int EventCounter() const;	
	u_int TriggerTimeTag() const;
	u_int Groups() const;
	u_int Channels() const;	
	

	u_int getGroupSizeInBuffer() const;
	int getGroupIndexInBuffer(u_int grp) const;
	void setGroupHeader(unsigned int grp);
	size_t SamplesPerChannel() const;


	//get functions
	const header* gethead() const;
	
	//int getGroupPositionInBuffer(unsigned int grp) const;
	//uint16_t getSample(unsigned int chan, unsigned int sample, const uint32_t * subbuffer) const;
	size_t getChannelData(unsigned int grp, unsigned int chan, uint16_t* array, size_t arraylen) const;
	//size_t getChannelVoltage(unsigned int grp, unsigned int chan, float* array, size_t arraylen, uint16_t dac_offset) const;
	//size_t getChannelData(unsigned int chan, uint16_t* array, size_t arraylen) const{
	//	return getChannelData(chan/8, chan%8, array, arraylen);}
	//size_t getChannelVoltage(unsigned int chan, float* array, size_t arraylen, uint16_t dac_offset) const{
	//	return getChannelVoltage(chan/8, chan%8, array, arraylen,dac_offset);}


	int rawdata_size() const;
	int resize_rawbuffer(int to_bytesize);
	
	
  private:	
	u_int	    *buffer;	
	int		    bufsize; //bytes
	union 		header head;
	union		gr_headers group_heads;
	union		gr_trailers group_trails;


};


#endif //VX1742EVENT_HH
