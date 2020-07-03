#include "cell-individual-offset.h"


std::vector<double> CellIndividualOffset::OffsetList(40,0);

void CellIndividualOffset::setOffsetList(std::vector<double>& CioList)
{
		OffsetList = CioList;
}


std::vector<double> CellIndividualOffset::getOffsetList()
{
		return OffsetList;
}
