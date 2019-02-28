#ifndef FILEPROCESSING_H
#define FILEPROCESSING_H

#include "Cell.h"

#include <string>

void processMapRuleFiles(char **argv, int argc, Cells *cells); //Arg1: pointer to an array of pointers to c-strings. Arg2: amount of elements in the array.
void addCellsFromStr(Cells &cells, std::string str);

#endif