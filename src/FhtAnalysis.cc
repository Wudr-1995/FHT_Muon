#include "EvtNavigator/NavBuffer.h"
#include "SniperKernel/AlgFactory.h"
#include "BufferMemMgr/IDataMemMgr.h"
#include "Event/RecHeader.h"
#include "Event/CalibHeader.h"
#include "Event/ElecHeader.h"
#include "Event/SimHeader.h"
#include "RootWriter/RootWriter.h"
#include "TTree.h"
#include "TVector3.h"
#include "TMath.h"
#include "firstHitTimeExpected.h"
#include "FhtAnalysis.h"

DECLARE_ALGORITHM(FhtAnalysis);

FhtAnalysis::FhtAnalysis(const std::string& name)
: AlgBase(name),
m_iEvt(0),
m_buf(0),
m_usedPmtNum(0)
{
	declProp("LSRaduius", m_LSRadius = 17700);
	declProp("WaterRadius", m_WaterRadius = 19500);
	declProp("LightSpeed", m_cLight = 299.792458);
	declProp("LSRefrection", m_nLS = 1.485);
	declProp("MuonSpeed", m_vMuon = 299.792458);
	declProp("Pmt3inchTimeReso", m_3inchRes = 1);
	declProp("Pmt20inchTimeReso", m_20inchRes = 8);
	declProp("Use3inchPmt", m_3inchusedflag = false);
	declProp("Use20inchPmt", m_20inchusedflag = true);
	declProp("RelativeTimeCut", m_relativetimecut = 500);
	declProp("ChargeCut", m_qcut = 0);
	declProp("NPmtRequired", m_PmtNumCut = 100);
}

bool FhtAnalysis::initialize() {
	LogDebug << "Initializing" << std::endl;
	if(not initGeomSvc())
		return false;
	SniperDataPtr<JM::NavBuffer> navBuf(getParent(), "/Event");
	if (navBuf.invalid()) {
		LogError << "Cannot get the NavBuffer @ /Event" << std::endl;
		return false;
	}
	m_buf = navBuf.data();

	SniperPtr<RootWriter> m_rw(*getParent(), "RootWriter");
	if (!m_rw.valid()) {
		LogError << "Cannot get the RootWriter instance" << std::endl;
		return false;
	}
	m_tree_fht = new TTree("Expections_vs_Predictions", "Expections_vs_Predictions");
	m_tree_trks = new TTree("Tracks", "Tracks");
	m_tree_fht->Branch("Expect_Time", &m_expectT, "expecT/D");
	m_tree_fht->Branch("Real_Time", &m_realT, "predicT/D");
	m_tree_fht->Branch("nPE", &m_pe, "nPE/D");
	m_tree_fht->Branch("PMT_Dis", &m_dis, "Dis/D");
	m_tree_fht->Branch("ti", &m_ti, "ti/D");
	m_tree_fht->Branch("dtc", &m_dtc, "dtc/D");
	m_tree_fht->Branch("dte", &m_dte, "dte/D");
	m_tree_fht->Branch("ci", &m_ci, "ci/D");
	m_tree_trks->Branch("Difference_FPMT_Start", &m_diff_fpmtstart, "Different_FPMT_Start/D");
	m_tree_trks->Branch("Start_x", &sx, "sx/D");
	m_tree_trks->Branch("Start_y", &sy, "sy/D");
	m_tree_trks->Branch("Start_z", &sz, "sz/D");
	m_tree_trks->Branch("End_x", &ex, "ex/D");
	m_tree_trks->Branch("End_y", &ey, "ey/D");
	m_tree_trks->Branch("End_z", &ez, "ez/D");
	
	m_rw->attach("USER_OUTPUT", m_tree_fht);
	m_rw->attach("USER_OUTPUT", m_tree_trks);
	return true;
}

bool FhtAnalysis::execute() {
	LogDebug << "executing: " << m_iEvt ++ << std::endl;
	JM::SimEvent* simevent = 0;
	JM::CalibEvent* calibevent = 0;

	JM::EvtNavigator* nav =m_buf->curEvt();
	std::vector<std::string>& paths = nav->getPath();
	std::vector<JM::SmartRef*>& refs = nav->getRef();
	JM::SimHeader* simheader = 0;
	for (size_t i = 0; i < paths.size(); ++i) {
		const std::string& path = paths[i];
		JM::SmartRef* ref = refs[i];
		JM::EventObject* evtobj = refs[i]->GetObject();
		/*
		if (path == "/Event/Sim") {
			simheader = dynamic_cast<JM::SimHeader*>(evtobj);
			std::cout << "SimHeader (/Event/Sim): " << simheader << std::endl;
			if (simheader)
				break;
		}
		*/
		if (path == "/Event/SimOrig") {
			simheader = static_cast<JM::SimHeader*>(nav->getHeader("/Event/SimOrig"));
			std::cout << "SimHeader (/Event/SimOrig): " << simheader << std::endl;
			if (simheader)
				break;
		}
	}
	if (initPmt())
		LogDebug << "Initializing PMT success" << std::endl;
	else {
		LogError << "Initializing PMT fails" << std::endl;
		return false;
	}
	if (freshPmtData())
		LogDebug << "Freshing PMT data success" << std::endl;
	else {
		LogError << "Freshing PMT data fails" << std::endl;
		return false;
	}
	if (m_usedPmtNum < m_PmtNumCut) {
		LogDebug << "The number of PMTs used is less than " << m_PmtNumCut << ", skip this event." << std::endl;
		return true;
	}
	simevent = (JM::SimEvent*)simheader->event();
	nSimTrks = simevent->getTracksVec().size();
	LogDebug << "Retrieving tracks data" << std::endl;
	for (unsigned int i = 0; i < nSimTrks; i ++) {
		JM::SimTrack* strk = simevent->findTrackByTrkID(i);
		TVector3 tmp;
		tmp.SetXYZ(strk->getInitX(), strk->getInitY(), strk->getInitZ());
		TVector3 dir;
		dir.SetXYZ(strk->getInitPx(), strk->getInitPy(), strk->getInitPz());
		tmp = GetInciPos(tmp, dir, m_LSRadius);
		TVector3 WPos = GetInciPos(tmp, dir, m_WaterRadius - 1800);
		m_pmtstart.push_back(WPos);
		dir.SetMag(1.);
		dir.SetMag(2 * (TVector3(0, 0, 0) - WPos) * dir);
		m_pmtend.push_back(WPos + dir);
		sx = tmp.X();
		sy = tmp.Y();
		sz = tmp.Z();
		if (tmp == TVector3(0, 0, 0))
			return true;
		tmp.SetMag(m_LSRadius);
		LogDebug << "Tracks: " << i 
			<< ", start position: (" << tmp.X() << ", " << tmp.Y() << ", " << tmp.Z() << ")" << std::endl;
		m_start.push_back(tmp);
		short fstpmt = FindFirstPMT(tmp);
		m_fpmtht.push_back(m_ptab[fstpmt].fht);
		m_diff_fpmtstart = (tmp - m_ptab[fstpmt].pos).Mag();

		dir.SetMag(1.);
		 LogDebug << "Tracks: " << i 
			<< ", direction: (" << dir.X() << ", " << dir.Y() << ", " << dir.Z() << ")" << std::endl;
		m_dir.push_back(dir);
		dir.SetMag(2 * (TVector3(0, 0, 0) - tmp) * dir);
		LogDebug << "Distance from track to center: "
			<< (TVector3(0, 0, 0) - (tmp + TVector3(dir.X() / 2, dir.Y() / 2, dir.Z() / 2))).Mag() << std::endl;
		tmp = tmp + dir;
		tmp.SetMag(m_LSRadius);
		ex = tmp.X();
		ey = tmp.Y();
		ez = tmp.Z();
		m_end.push_back(tmp);
		fstpmt = FindFirstPMT(tmp);
		m_tree_trks->Fill();

		LogDebug << "Out incident position: (" << m_pmtstart[i].Theta() << ", " << m_pmtstart[i].Phi() << ")" << std::endl;
		LogDebug << "Out incident position: (" << m_start[i].Theta() << ", " << m_start[i].Phi() << ")" << std::endl;

		LogDebug << "LengthInner: " << (m_start[i] - m_end[i]).Mag() << std::endl;
		LogDebug << "LengthOuter: " << (m_pmtstart[i] - m_pmtend[i]).Mag() << std::endl;
	}
	PmtCut();
	LogDebug << "Retrieving first hit time data." << std::endl;
	LogDebug << "Estimating first hit time data." << std::endl;
	for (unsigned int i = 0; i < tot_pmtnum; i ++) {
		if (m_ptab[i].used) {
			double* tmp;
			m_realT = m_ptab[i].fht;
			tmp = PredictFht(m_ptab[i]);
			m_expectT = tmp[0];
			m_ti = tmp[1];
			m_dtc = tmp[2];
			m_ci = tmp[3];
			m_dte = tmp[3];
			m_pe = m_ptab[i].q;
			m_dis = GetDisPMT2Trk(m_ptab[i]);
			// LogDebug << "Real fht: " << m_realT << ", Expect fht: " << m_expectT << std::endl;
			m_tree_fht->Fill();
		}
	}
	return true;
}

double FhtAnalysis::GetDisPMT2Trk(PmtProp& pmt) {
	TVector3 tmp;
	double out;
	for (short int i = 0; i < m_start.size(); i ++) {
		tmp = m_dir[i];
		tmp.SetMag(abs((m_start[i] - pmt.pos) * m_dir[i]));
		double dis = (pmt.pos - (m_start[i] + tmp)).Mag();
		// LogDebug << "tmp: (" << tmp.X() << ", " << tmp.Y() << ", " << tmp.Z() << ")" << std::endl;

		// LogDebug << "Distance: " << dis << std::endl;
		// LogDebug << "PMT Position: (" << pmt.pos.X() << ", " << pmt.pos.Y() << ", " << pmt.pos.Z() << ")" << std::endl;
		// LogDebug << "Incident Position: (" << m_start[i].X() << ", " << m_start[i].Y() << ", " << m_start[i].Z() << ")" << std::endl;
		// LogDebug << "Direction: (" << m_dir[i].X() << ", " << m_dir[i].Y() << ", " << m_dir[i].Z() << ")" << std::endl;


		if (!i)
			out = dis;
		else if (out > dis)
			out = dis;
	}
	return out;
}

double FhtAnalysis::FindFirstPMT(TVector3 ri) {
	double out;
	out = 0;
	for (int i = 0; i < tot_pmtnum; i ++) {
		if ((ri - m_ptab[i].pos).Mag() < (ri - m_ptab[out].pos).Mag() && m_ptab[i].used) {
			// LogDebug << "Fht of PMTs: " << m_ptab[i].fht << " Num: "<< i << std::endl;
			out = i;
			// LogDebug << "Status: " << out << std::endl;
		}
	}
	LogDebug << "First Hit Time of This Track: " << m_ptab[out].fht << " Num: " << out << std::endl;
	LogDebug << "First Hit PMT's Position: (" << m_ptab[out].pos.X() << ", " << m_ptab[out].pos.Y() << ", " << m_ptab[out].pos.Z() << ")" << std::endl;
	return out;
}

double* FhtAnalysis::PredictFht(PmtProp& pos) {
	Params pars;
	pars.set("LSRefraction", m_nLS);
	pars.set("MuonSpeed", m_vMuon);
	pars.set("LightSpeed", m_cLight);
	// pars.set("WaterRefraction", 1.34);
	double* tmp;
	double* out;
	for (unsigned int i = 0; i < m_start.size(); i ++) {
		// tmp = firstHitTimeExpected(m_start[i], m_pmtstart[i], m_fpmtht[i], m_dir[i], (m_end[i] - m_start[i]).Mag(), (m_pmtend[i] - m_pmtstart[i]).Mag(), pos, pars);
		tmp = firstHitTimeExpected(m_pmtstart[i], m_fpmtht[i], m_dir[i], (m_pmtend[i] - m_pmtstart[i]).Mag(), pos, pars);
		if (i == 0)
			out = tmp;
		else if (tmp[0] < out[0])
			out = tmp;
		else ;
	}
	return out;
}

bool FhtAnalysis::initPmt() {
	LogDebug << "Initializing PMTs" << std::endl;
	tot_pmtnum = 0;
	tot_pmtnum = m_geom->getPmtNum();
	if (!tot_pmtnum) {
		LogError << "Wrong PMT Number" << std::endl;
		return false;
	}
	LogDebug << "PMT Number Got" << std::endl;
	m_ptab.reserve(tot_pmtnum);
	m_ptab.resize(tot_pmtnum);
	for (unsigned int pid = 0; pid < tot_pmtnum; pid ++) {
		Identifier Id = Identifier(CdID::id(pid, 0));
		PmtGeom* pmt = m_geom->getPmt(Id);
		if (!pmt) {
			LogError << "Wrong PMT ID" << std::endl;
			return false;
		}
		TVector3 pmtCenter = pmt->getCenter();
		m_ptab[pid].pos = pmtCenter;
		if (CdID::is3inch(Id)) {
			m_ptab[pid].res = m_3inchRes;
			m_ptab[pid].type = _PMTINCH3;
		}
		else if (CdID::is20inch(Id)) {
			m_ptab[pid].res = m_20inchRes;
			m_ptab[pid].type = _PMTINCH20;
		}
		else {
			LogError << "Pmt[" << pid << "] is neither 3-inch or 20-inch" << std::endl;
			return false;
		}
		m_ptab[pid].q = -1;
		m_ptab[pid].fht = 99999;
		m_ptab[pid].used = false;
	}
	return true;
}

bool FhtAnalysis::freshPmtData() {
	JM::EvtNavigator* nav = m_buf->curEvt();
	if (not nav) {
		LogError << "Cannot retrieve current navigator" << std::endl;
		return false;
	}
	JM::CalibHeader* calibheader = (JM::CalibHeader*)nav->getHeader("/Event/Calib");
	if (not calibheader) {
		LogError << "Cannot retrieve '/Event/Calib'" << std::endl;
		return false;
	}
	const std::list<JM::CalibPMTChannel*>& chhlist = calibheader->event()->calibPMTCol();
	std::list<JM::CalibPMTChannel*>::const_iterator chit = chhlist.begin();
	if (chit == chhlist.end())
		LogDebug << "chit == chhlist.end()" << std::endl;
	if (chit != chhlist.end()) {
		JM::CalibPMTChannel* tmp = *chit;
		m_globalfht = tmp->firstHitTime();
	}
	while (chit != chhlist.end()) {
		JM::CalibPMTChannel* calib = *chit ++;
		Identifier id = Identifier(calib->pmtId());
		Identifier::value_type value = id.getValue();
		if (not (value & 0xFF000000) >> 24 == 0x10)
			continue;
		unsigned int pid = CdID::module(id);
		if (pid > tot_pmtnum) {
			LogError << "Data/Geometry Mis-Match : PmtId(" << pid << ") >= the number of PMTs." << std::endl;
			return false;
		}
		m_ptab[pid].q = calib->nPE();
		m_ptab[pid].fht = calib->firstHitTime();
		if (calib->firstHitTime() < m_globalfht) {
			m_globalfht = calib->firstHitTime();
			m_fhpos = m_ptab[pid].pos;
		}
		if ((CdID::is3inch(id) && m_3inchusedflag || CdID::is20inch(id) && m_20inchusedflag) && calib->nPE() > m_qcut) {
			m_ptab[pid].used = true;
			m_usedPmtNum ++;
		}
	}
	LogDebug << "Loading calibration data done" << std::endl;
	return true;
}

bool FhtAnalysis::initGeomSvc() {
	SniperPtr<RecGeomSvc> rgSvc(getParent(), "RecGeomSvc");
	if (rgSvc.invalid()) {
		LogError << "Failed to get RecGeomSvc instance" << std::endl;
		return false;
	}
	m_geom = rgSvc->getCdGeom();
	return true;
}

bool FhtAnalysis::PmtCut() {
	for (unsigned int i = 0; i < tot_pmtnum; i ++) {
		if (m_ptab[i].fht - m_globalfht > m_relativetimecut) {
			m_ptab[i].used = false;
			m_usedPmtNum --;
		}
	}
	return true;
}

bool FhtAnalysis::finalize() {
	LogDebug << "Finalizing" << std::endl;
	return true;
}

TVector3 FhtAnalysis::GetInciPos(TVector3 pos, TVector3 dir, double R) {
	dir.SetMag(1.);
	dir.SetMag((TVector3(0, 0, 0) - pos) * dir);
	LogDebug << "dir ori: (" << dir.X() << ", " << dir.Y() << ", " << dir.Z() << ")" << std::endl;
	TVector3 tmp = pos + dir;
	double dis = (TVector3(0, 0, 0) - tmp).Mag();
	LogDebug << "dis: " << dis << std::endl;
	if (R < dis) {
		LogDebug << "The track didn't through CD" << std::endl;
		return TVector3(0, 0, 0);
	}
	double halfstr = pow((pow(R, 2) - pow(dis, 2)), 0.5);
	LogDebug << "halfstr: " << halfstr << std::endl;
	dir.SetMag(dir.Mag() - halfstr);
	return (pos + dir);
}
