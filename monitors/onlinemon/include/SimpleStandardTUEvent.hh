/* ---------------------------------------------------------------------------------
** OSU Trigger Logic Unit EUDAQ Implementation
** 
**
** <SimpleStandardTUEvent>.hh
** 
** Date: April 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

#ifndef SimpleStandardTUEvent_HH
#define SimpleStandardTUEvent_HH



class SimpleStandardTUEvent{
protected:
	std::string name;


public:
	SimpleStandardTUEvent(const std::string &name, const int id, const unsigned int nsamples, OnlineMonConfiguration* mymon);
	SimpleStandardTUEvent(const std::string &name, const int id, const unsigned int nsamples);
	virtual ~SimpleStandardTUEvent() {};


private:
	OnlineMonConfiguration* mon;
};

#endif //SimpleStandardTUEvent_HH