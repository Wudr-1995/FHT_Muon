#ifndef FhtAnalysis_h
#define FhtAnalysis_h

#include "SniperKernel/AlgBase.h"
#include "EvtNavigator/NavBuffer.h"
#include "RootWriter/RootWriter.h"
#include "Identifier/Identifier.h"
#include "Identifier/CdID.h"
#include "Geometry/RecGeomSvc.h"
#include "PmtProp.h"
#include "Params.h"
#include "TTree.h"
#include "TVector3.h"
#include "TMath.h"
#include <fstream>
#include <iostream>

class CdGeom;

class FhtAnalysis : public AlgBase{
	public :
		FhtAnalysis(const std::string& name);

		bool initialize();
		bool execute();
		double FindFirstPMT(TVector3);
		bool initGeomSvc();
		bool initPmt();
		bool freshPmtData();
		bool PmtCut();
		bool finalize();
		double* PredictFht(PmtProp& pos);
		double GetDisPMT2Trk(PmtProp&);
		TVector3 GetInciPos(TVector3, TVector3, double);
	private :
		int m_iEvt;
		unsigned int tot_pmtnum;
		JM::NavBuffer* m_buf;
		TTree* m_tree_fht;
		TTree* m_tree_trks;
		TVector3 m_fhpos;
		std::vector<TVector3> m_start, m_end, m_dir, m_pmtstart, m_pmtend;
		std::vector<double> m_fpmtht;
		int nSimTrks;
		double m_expectT, m_realT;
		double sx, sy, sz, ex, ey, ez;
		double Mid_dis;

		bool m_3inchusedflag;
		bool m_20inchusedflag;

		double m_LSRadius;
		double m_WaterRadius;
		double m_cLight;
		double m_vMuon;
		double m_nLS;
		double m_3inchRes;
		double m_20inchRes;
		double m_relativetimecut;
		double m_qcut;
		double m_globalfht;
		double m_diff_fpmtstart;
		double m_pe;
		double m_dis;
		double m_ti, m_dtc, m_ci, m_dte;
		int m_usedPmtNum;
		int m_PmtNumCut;
		PmtTable m_ptab;
		CdGeom* m_geom;
};
#endif
