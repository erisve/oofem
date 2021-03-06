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
 *               Copyright (C) 1993 - 2013   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "sm/EngineeringModels/xfemsolverinterface.h"
#include "sm/Elements/structuralelement.h"
#include "sm/Materials/InterfaceMaterials/structuralinterfacematerial.h"
#include "sm/Materials/InterfaceMaterials/structuralinterfacematerialstatus.h"
#include "sm/xfem/xfemstructuralelementinterface.h"
#include "sm/mappers/primvarmapper.h"
#include "timestep.h"
#include "structengngmodel.h"
#include "staticstructural.h"
#include "primaryfield.h"
#include "domain.h"
#include "xfem/xfemmanager.h"
#include "element.h"
#include "matstatmapperint.h"
#include "nummet.h"
#include "floatarray.h"
#include "exportmodulemanager.h"
#include "vtkxmlexportmodule.h"
#include "dynamicinputrecord.h"

namespace oofem {

XfemSolverInterface::XfemSolverInterface():
mNeedsVariableMapping(false),
mForceRemap(false)
{

}

XfemSolverInterface::~XfemSolverInterface()
{

}

void XfemSolverInterface::propagateXfemInterfaces(TimeStep *tStep, StructuralEngngModel &ioEngngModel, bool iRecomputeStepAfterCrackProp)
{
    int domainInd = 1;
    Domain *domain = ioEngngModel.giveDomain(domainInd);


    if(domain->hasXfemManager()) {
    	// Dirty debug output

        XfemManager *xMan = domain->giveXfemManager();

        int numEI = xMan->giveNumberOfEnrichmentItems();
        for(int i = 1; i <= numEI; i++) {
        	EnrichmentItem *ei = xMan->giveEnrichmentItem(i);
        	ei->writeVtkDebug(i);
        }

    }


    bool frontsHavePropagated = false;

    if(domain->hasXfemManager()) {
        XfemManager *xMan = domain->giveXfemManager();
        if ( xMan->hasInitiationCriteria() ) {
            // TODO: generalise this?
            // Intitiate delaminations (only implemented for listbasedEI/delamination. Treated the same way as propagation)
            xMan->initiateFronts(frontsHavePropagated,tStep);
        }

        if( xMan->hasPropagatingFronts() ) {
            // Propagate crack tips
            xMan->propagateFronts(frontsHavePropagated);

        }

        if(frontsHavePropagated || mForceRemap) {

			int numEl = domain->giveNumberOfElements();
			for ( int i = 1; i <= numEl; i++ ) {
//				printf("XfemSolverInterface::propagateXfemInterfaces: Calling XfemElementInterface_updateIntegrationRule.\n");

				////////////////////////////////////////////////////////
				// Map state variables for enriched elements
				XfemElementInterface *xfemElInt = dynamic_cast< XfemElementInterface * >( domain->giveElement(i) );

				if(xfemElInt) {
					xfemElInt->XfemElementInterface_updateIntegrationRule();
				}

			}


            mNeedsVariableMapping = false;

            if( frontsHavePropagated ) {
				ioEngngModel.giveDomain(1)->postInitialize();
				ioEngngModel.forceEquationNumbering();
            }

            if(iRecomputeStepAfterCrackProp) {
                printf("Recomputing time step.\n");
                ioEngngModel.forceEquationNumbering();
                ioEngngModel.solveYourselfAt(tStep);
                ioEngngModel.updateYourself( tStep );
                ioEngngModel.terminate( tStep );

            }
        }
    }






    if(domain->hasXfemManager()) {
        XfemManager *xMan = domain->giveXfemManager();

        bool eiWereNucleated = false;
        if( xMan->hasNucleationCriteria() ) {

        	if(!frontsHavePropagated) {
        		xMan->nucleateEnrichmentItems(eiWereNucleated);
        	}
       }

        if( eiWereNucleated || mForceRemap) {

			int numEl = domain->giveNumberOfElements();
			for ( int i = 1; i <= numEl; i++ ) {
//				printf("XfemSolverInterface::propagateXfemInterfaces: Calling XfemElementInterface_updateIntegrationRule.\n");

				////////////////////////////////////////////////////////
				// Map state variables for enriched elements
				XfemElementInterface *xfemElInt = dynamic_cast< XfemElementInterface * >( domain->giveElement(i) );

				if(xfemElInt) {
					xfemElInt->XfemElementInterface_updateIntegrationRule();
				}

			}


            mNeedsVariableMapping = false;

            if( eiWereNucleated ) {
				ioEngngModel.giveDomain(1)->postInitialize();
				ioEngngModel.forceEquationNumbering();
            }


            
            if(iRecomputeStepAfterCrackProp) {
                printf("Recomputing time step.\n");
                ioEngngModel.forceEquationNumbering();
                ioEngngModel.solveYourselfAt(tStep);
                ioEngngModel.updateYourself( tStep );
                ioEngngModel.terminate( tStep );

            }
        }
    }

}

IRResultType
XfemSolverInterface :: initializeFrom(InputRecord *ir)
{

//	printf("Entering XfemSolverInterface :: initializeFrom(InputRecord *ir).\n");

	mForceRemap = ir->hasField(_IFT_XfemSolverInterface_ForceRemap);

//	if(mForceRemap) {
//		printf("mForceRemap is true.\n");
//	}
//	else {
//		printf("mForceRemap is false.\n");
//	}


    return IRRT_OK;
}


} /* namespace oofem */
