#include "DebugBreak.h"


CDebugBreak::CDebugBreak(Mass* srcMass)
	:m_SrcMass(srcMass)
{
	assert(srcMass);
	m_ID = srcMass->m_ID;
}

CDebugBreak::~CDebugBreak(){
	if (m_SrcMass)
	{
		delete m_SrcMass;
		m_SrcMass = NULL;
	}
}
MASS_TYPE  CDebugBreak::MassType(){
	return MASS_USER;
}; 

TypeAB CDebugBreak::GetTypeAB(){ 
	assert(m_SrcMass);
	return m_SrcMass->GetTypeAB();
};

tstring CDebugBreak::GetName(){
	assert(m_SrcMass);
	return m_SrcMass->GetName();
};

bool  CDebugBreak::Do(Energy* E){
	assert(m_SrcMass);
	ePipeline* Pipe = (ePipeline*)E;
	Pipe->SetID(RETURN_BREAK);
	bool ret = m_SrcMass->Do(E);
	return ret;
};

Mass* CDebugBreak::Release(){
	Mass* m = m_SrcMass;
	m_SrcMass = NULL;
	return m;
}
