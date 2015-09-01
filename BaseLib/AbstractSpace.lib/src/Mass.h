#ifndef _MASS_H_
#define _MASS_H_

#include "AbstractSpace.h"

namespace ABSTRACT { 


enum  MASS_TYPE {
    MASS_VOID        =    0,  
	MASS_ELEMENT     =    1,  
	MASS_OBJECT      =    2,
	MASS_MODEL       =    3,
	MASS_SYSTEM      =    4,
	MASS_USER        =    5
};

class Energy;

class  Mass : public AbstractSpace  
{
protected: 
	Mass(CABTime* Timer,CABSpacePool* Pool):
	AbstractSpace(Timer,Pool)
	{
	}
	//hardly use for the mass class' instances,so weakened it as private 
    //virtual bool  Do(AbstractSpace* Other){ return true;};
public:
	int64    m_ID;
public:
	Mass():m_ID(0){};
	Mass(int64 ID):m_ID(ID){};
	virtual ~Mass(){};

    virtual SPACE_TYPE SpaceType(){ return MASS_SPACE;};
	virtual MASS_TYPE  MassType(){ return MASS_VOID;}; 

    virtual Energy*  ToEnergy(); 
	virtual bool     FromEnergy(Energy* E); 

	/*The interaction between mass and energy,but from a programmer's point of view,
	 Do(Energy* E) like a unified form of arbitrary c functions whose parameters provided by E
	*/
	virtual bool Do(Energy* E)=0;  

	/*returns the energies abbreviation(the parameters list) that the mass can accept,
	 reference to the specific instance of this class and Pipeline.h
	*/
	virtual TypeAB   GetTypeAB()=0;  
};

}//end namespace 

#endif
