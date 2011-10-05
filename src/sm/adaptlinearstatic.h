/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2011   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef adaptlinearstatic_h
#define adaptlinearstatic_h

#include "linearstatic.h"
#include "meshpackagetype.h"

namespace oofem {

/**
 * This class implements an adaptive linear static engineering problem.
 * Multiple loading cases are not supported.
 * Due to linearity of a problem, the complete reanalysis from the beginning
 * is done after adaptive remeshing.
 * Solution steps represent a series of adaptive analyses.
 */
class AdaptiveLinearStatic : public LinearStatic
{
protected:
    /// Error estimator used for determining the need for refinements.
    ErrorEstimator *ee;
    /// Meshing package used for refinements.
    MeshPackageType meshPackage;

public:
    AdaptiveLinearStatic(int i, EngngModel *_master = NULL) : LinearStatic(i, _master) { ee = NULL; }
    ~AdaptiveLinearStatic() { }

    void solveYourselfAt(TimeStep *tStep);

    /**
     * Initializes the newly generated discretization state according to previous solution.
     * This process should typically include restoring old solution, instanciating newly
     * generated domain(s) and by mapping procedure.
     */
    virtual int initializeAdaptive(int stepNumber);

    virtual contextIOResultType restoreContext(DataStream *stream, ContextMode mode, void *obj = NULL);

    void updateDomainLinks();

    IRResultType initializeFrom(InputRecord *ir);

    ErrorEstimator *giveDomainErrorEstimator(int n) { return ee; }

    // identification
    const char *giveClassName() const { return "AdaptiveLinearStatic"; }
    classType giveClassID() const { return AdaptiveLinearStaticClass; }
};
} // end namespace oofem
#endif // adaptlinearstatic_h
