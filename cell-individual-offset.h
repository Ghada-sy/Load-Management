
#ifndef CELL_INDIVIDUAL_OFFSET_H
#define CELL_INDIVIDUAL_OFFSET_H

#include <vector>

class CellIndividualOffset
{


	
	static std::vector<double> OffsetList;
	public:	
		static void setOffsetList(std::vector<double>& CioList);
		static std::vector<double> getOffsetList();


};

#endif
