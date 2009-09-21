#include "debug.h"

#include <iostream>
#include <iomanip>
#include <fstream>


namespace ibus
{
    using namespace std;
    
    std::fstream log("/tmp/ibus.log", fstream::app|fstream::out);
}
