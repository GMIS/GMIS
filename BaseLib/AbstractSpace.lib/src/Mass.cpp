#include "Mass.h"
#include "Energy.h"
#include "BaseEnergy.h"

namespace ABSTRACT{


Energy*  Mass::ToEnergy(){
	return new eNULL;
}; 

bool Mass::FromEnergy(Energy* E){
	assert(E->EnergyType() == TYPE_NULL);
	return true;
}

}//end namespace