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


//#include <sys/types.h>


class VX1742Event{
  
  public:
	union header_size {
	    struct {
		  u_int A					:4;
		  u_int eventSize		:28;
	  };
		u_int raw;
	};


	union header_pattern {
	    struct{
		  u_int board_id			:5;
		  u_int reserved_0		:5;
		  u_int pattern			:14;
		  u_int reserved_1		:4;
		  u_int group_mask		:4;
		};
		u_int raw;
	};

	union header_event_counter {
		struct {
		  u_int reserved			:10;
		  u_int event_counter	:22;  
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

	
	VX1742Event();
	virtual ~VX1742Event();
/*

	int setData(st_header* newhead, const u_int* newbuffer, int newbuflen);
	void invalidate();
	bool isValid() const;
	u_int EventSize() const;	
	u_int BoardID() const;
	u_int Pattern() const;		
	u_int GroupMask() const;
	u_int EventCounter() const;	
	u_int TriggerTimeTag() const;
	size_t SamplesPerChannel() const;
	u_int Channels() const;


	int rawdata_size() const;
	const st_header* gethead() const;
	size_t getChannelData(unsigned int grp, unsigned int chan, u_int* array, size_t arraylen) const;
	size_t getChannelVoltage(unsigned int grp, unsigned int chan, float* array, size_t arraylen, u_int dac_offset) const;
	size_t getChannelData(unsigned int chan, u_int* array, size_t arraylen) const{
		return getChannelData(chan/8, chan%8, array, arraylen);}
	size_t getChannelVoltage(unsigned int chan, float* array, size_t arraylen, u_int dac_offset) const{
		return getChannelVoltage(chan/8, chan%8, array, arraylen,dac_offset);}
	*/

  private:	
	u_int	    *buffer;	
	int		    bufsize;	
	union 		header head;

	/*
	int resize_rawbuffer(int to_bytesize);
	int getGrpPos(unsigned int grp) const;
	u_int getSample(unsigned int chan, unsigned int sample, const u_int * subbuffer) const;	
	*/


};


#endif //VX1742EVENT_HH
