#include "firstHitTimeExpected_wdr.h"
#include "TMath.h"
	double* 
firstHitTimeExpected(TVector3 ri,
		TVector3 wi,
		double twi,
		TVector3 direction, 
		double lengthinner,
		double lengthouter,
		const PmtProp& pmt,
		const Params& params
		) 
{
	double m_nLSlight = params.get("LSRefraction");
	double m_clight = params.get("LightSpeed");
	double m_vmuon = params.get("MuonSpeed");
	double m_nWater = params.get("WaterRefraction");
	
	double tanthetaW = TMath::Sqrt(m_nWater * m_nWater - 1);
	double tanthetaLS = TMath::Sqrt(m_nLSlight * m_nLSlight - 1);

	TVector3 Outperp = wi + direction * (pmt.pos - wi) * direction;
	// TVector3 Innperp = ri + direction * (pmt.pos - ri) * direction;
	double DisW = (Outperp - wi).Mag() - (pmt.pos - Outperp).Mag() / tanthetaW;
	double DisLS = (Outperp - wi).Mag() - (pmt.pos - Outperp).Mag() / tanthetaLS;
	TVector3 LightPos;
	double D = (lengthouter - lengthinner) / 2;
	double n;
	if (DisW <= 0) {
		LightPos = wi + direction * D;
		n = m_nWater;
	}
	else if (DisW > 0 && DisW <= D) {
		if (DisLS <= D) {
			LightPos = wi + direction * DisW;
			n = m_nWater;
		}
		else {
			LightPos = wi + direction * DisLS;
			n = m_nLSlight;
		}
	}
	else if (DisW > D && DisW <= D + lengthinner) {
		if (DisLS <= D + lengthinner) {
			LightPos = wi + direction * DisLS;
			n = m_nLSlight;
		}
		else {
			LightPos = wi + direction * (D + lengthinner);
			n = m_nLSlight;
		}
	}
	else if (DisW > D + lengthinner && DisW <= lengthouter) {
		LightPos = wi + direction * DisW;
		n = m_nWater;
	}
	else {
		LightPos = wi + direction * (D + lengthinner);
		n = m_nWater;
	}

	double fht = twi + (LightPos - wi).Mag() / m_vmuon + (pmt.pos - LightPos).Mag() / m_clight * n;
	// double out[4] = {fht, twi, (LightPos - wi).Mag() / m_vmuon, (pmt.pos - LightPos).Mag() / m_clight * n};
	double out[4] = {fht, (Outperp - wi).Mag(), DisLS, DisW};

	double* p;
	p = out;
	return p;
}
