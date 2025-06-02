#include <fstream>
#include <map>


using namespace std;


bool LoadCDMConfiguration( map<std::string,std::string> &config );
std::vector<std::vector<int>>  LoadLedLoc();
std::vector<std::vector<int>>  LoadOARLLoc();