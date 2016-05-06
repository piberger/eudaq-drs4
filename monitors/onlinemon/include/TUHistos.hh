/* ---------------------------------------------------------------------------------
** OSU Trigger Logic Unit EUDAQ Implementation
** 
**
** <TUHistos>.hh
** 
** Date: May 2016
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** ---------------------------------------------------------------------------------*/

#ifndef TUHISTOS_HH_
#define TUHISTOS_HH_

class TH1I;
class SimpleStandardEvent;
class RootMonitor;


class TUHistos{

  protected:
    TH1I* _CoincidenceCount;


  public:
    TUHistos();
    virtual ~TUHistos();
    void Fill( SimpleStandardEvent ev);
    void Write();
    void Reset();
    TH1I*  getCoincidenceCountHisto(){return _CoincidenceCount;}



};

#ifdef __CINT__
#pragma link C++ class TUHistos-;
#endif

#endif
