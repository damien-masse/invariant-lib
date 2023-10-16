#ifndef __DIFF_EXPBOX_H__
#define __DIFF_EXPBOX_H__

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <vector>
#include <list>
#include <cmath>
#include <variant>
#include <ibex.h>
#include "expbox_util.h"

using namespace ibex;

namespace invariant {

class ExpVF {
   public:
     ExpVF (const IntervalVector &Box, const IntervalVector &VF, 
                        const std::pair<Matrix, IntervalVector>& pair);
     ExpVF (const IntervalVector &VF);
     ExpVF (const ExpVF &VF, double sens);
    
     const IntervalVector& getVF() const;
     void contract_flow(const ExpPoly &Start, ExpPoly &End) const;

   private :
     unsigned int dim; /* dimension of the space */
     bool constant_field; /* is A = 0 ? */

     IntervalVector VF;
     Vector C;      /* center of the box (needed if A!=0) */
     Matrix A;           /* room : dynamics */
     Vector Z1;          /* mean direction of VF (on the center)  */

     IntervalVector V;     /* room : dynamics : Xdot = A(X-C)+Z_1+V */
};

inline const IntervalVector& ExpVF::getVF() const {
     return this->VF;
}

}

#endif

