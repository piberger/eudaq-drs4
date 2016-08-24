/*
 * HitmapHistos.hh
 *
 *  Created on: Jun 16, 2011
 *      Author: stanitz
 */

#ifndef HITMAPHISTOS_HH_
#define HITMAPHISTOS_HH_

#include <TH2I.h>
#include <TFile.h>

#include <map>

#include "SimpleStandardEvent.hh"

using namespace std;

class RootMonitor;

class HitmapHistos {
  protected:
    string _sensor;
    int _id;
    int _maxX;
    int _maxY;
    bool _wait;
    std::map<std::string, TH1*> _histoMap;
    TH2I * _hitmap;
    TH2F * _chargemap;
    TH1I * _hitXmap;
    TH1I * _hitYmap;
    TH2I * _clusterMap;
    TH2I * _calMap;
    TH2I * _bgMap;
    TH1F * _hCalEff;
    TH1F * _hBgRate;
    TH2D * _HotPixelMap;
    TH1I * _lvl1Distr;
    TH1I * _lvl1Width;
    TH1I * _lvl1Cluster;
    TH1I * _totSingle;
    TH1I * _totCluster;
    TH1F * _hitOcc;
    TH1I * _clusterSize;
    TH1I * _clusterSizeBg;
    TH1I * _nClusters;
    TH1I * _nHits;
    TH1I * _clusterXWidth;
    TH1I * _clusterYWidth;
    TH1I * _nbadHits;
    TH1I * _nHotPixels;
    TH1I * _nPivotPixel;
    TH1I * _hitmapSections;
    TH1I ** _nHits_section;
    TH1I ** _nClusters_section;
    TH1I ** _nClustersize_section;
    TH1I ** _nHotPixels_section;
    TProfile * _efficencyPerEvent;
    TProfile * _clusterChargeProfile;
    TProfile * _pixelChargeProfile;
    ULong64_t _start_time;
    unsigned _eventNumber;
    unsigned _timestamp;
//    TH2D* _nTriggerPhase;
    void fillMimosaHistos(const SimpleStandardPlane *simpPlane);
    void loopOverPixelHits(const SimpleStandardPlane *simpPlane);
    void loopOverClusterHits(const SimpleStandardPlane *simpPlane);


  public:
    HitmapHistos(SimpleStandardPlane p, RootMonitor * mon);

    void Fill(const SimpleStandardHit & hit, int ntrig = 10, bool last=false);
    void Fill(const SimpleStandardPlane & plane, unsigned event_no, unsigned time_stamp);
    void Fill(const SimpleStandardCluster & cluster);
    void Reset();

    void Calculate(const int currentEventNum);
    void Write();
    TH1  * getHisto(std::string name);
    TH2I * getHitmapHisto() { return _hitmap; }
    TH2I * getCalMapHisto() { return _calMap; }
    TH2I * getBgMapHisto() { return _bgMap; }
    TH2F * getChargemapHisto() { return _chargemap; }
    TH1I * getHitXmapHisto() { return _hitXmap; }
    TH1I * getHitYmapHisto() { return _hitYmap; }
    TH1I * getHitmapSectionsHisto() { return _hitmapSections; }
    TH2I * getClusterMapHisto() { return _clusterMap; }
    TH2D * getHotPixelMapHisto() { return _HotPixelMap; }
    TH1I * getLVL1Histo() { return _lvl1Distr; }
    TH1I * getLVL1WidthHisto() { return _lvl1Width; }
    TH1I * getLVL1ClusterHisto() { return _lvl1Cluster; }
    TH1I * getTOTSingleHisto() { return _totSingle; }
    TH1I * getTOTClusterHisto() { return _totCluster; }
    TH1F * getHitOccHisto() { if (_wait) return NULL; else return _hitOcc; }
    TH1I * getClusterSizeHisto() { return _clusterSize; }
    TH1I * getClusterSizeBgHisto() { return _clusterSizeBg; }
    TH1I * getNHitsHisto() { return _nHits; }
    TH1I * getNClustersHisto() { return _nClusters; }
    TH1I * getClusterWidthXHisto() { return _clusterXWidth; }
    TH1I * getClusterWidthYHisto() { return _clusterYWidth; }
    TH1I * getNbadHitsHisto() { return _nbadHits; }
    TH1I * getSectionsNHitsHisto(unsigned int section) { return _nHits_section[section]; }
    TH1I * getSectionsNClusterHisto(unsigned int section) { return _nClusters_section[section]; }
    TH1I * getSectionsNClusterSizeHisto(unsigned int section) { return _nClustersize_section[section]; }
    TH1I * getSectionsNHotPixelsHisto(unsigned int section) { return _nHotPixels_section[section]; }
    TH1I * getNHotPixelsHisto() { return _nHotPixels; }
    TH1I * getNPivotPixelHisto(){ return _nPivotPixel;}
    TProfile* getEfficencyPerEvent(){return _efficencyPerEvent;}
    TProfile* getClusterChargeProfile(){return _clusterChargeProfile;}
    TProfile* getPixelChargeProfile(){return _pixelChargeProfile;}
    TH1F* getCalEff(){return _hCalEff;}
    TH1F* getBgRate(){return _hBgRate;}
//    TH2D * getTriggerPhaseHisto(){ return _nTriggerPhase;}
    void setRootMonitor(RootMonitor *mon)  {_mon = mon; }

    int ntrig;

  private:
    int ** plane_map_array; //store an array representing the map
    int zero_plane_array(); // fill array with zeros;
    int SetHistoAxisLabelx(TH1* histo,string xlabel);
    int SetHistoAxisLabely(TH1* histo,string ylabel);
    int SetHistoAxisLabels(TH1* histo,string xlabel, string ylabel);
    int filling_counter; //we don't need occupancy to be refreshed for every single event

    RootMonitor * _mon;
    unsigned int mimosa26_max_section;
    //check what kind sensor we're dealing with
    // for the filling this eliminates a string comparison
    bool is_MIMOSA26;
    bool is_APIX;
    bool is_USBPIX;
    bool is_USBPIXI4;
    bool is_DEPFET;
    bool is_CMSPIXEL;

};

#ifdef __CINT__
#pragma link C++ class HitmapHistos-;
#endif

#endif /* HITMAPHISTOS_HH_ */

