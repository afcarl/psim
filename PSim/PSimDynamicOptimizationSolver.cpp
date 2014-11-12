/* -------------------------------------------------------------------------- *
 *                    OpenSim:  PSimDynamicOptimizationSolver.cpp             *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2014 Stanford University and the Authors                *
 * Author(s): Chris Dembia                                                    *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

#include "PSimDynamicOptimizationSolver.h"

#include "StatesCollector.h"
#include "StateTrajectory.h"
#include "PSimTool.h"

#include <OpenSim/Simulation/Manager/Manager.h>

using namespace OpenSim;

PSimDynamicOptimizationSolver::PSimDynamicOptimizationSolver()
{
    constructProperties();
}

void PSimDynamicOptimizationSolver::constructProperties()
{
    constructProperty_optimization_convergence_tolerance(1e-4);
}

PSimParameterValueSet PSimDynamicOptimizationSolver::extendSolve(
        const PSimTool& tool) const
{
    // Get initial guess, and parameter limits.
    // ========================================
    SimTK::Vector results;
    SimTK::Vector lower;
    SimTK::Vector upper;
    tool.initialOptimizerParameterValuesAndLimits(results, lower, upper);

    // Setup the solver.
    // =================
    OptimizerSystem optsys(tool);
    optsys.setParameterLimits(lower, upper);

    // Create an Optimizer.
    // ====================
    SimTK::Optimizer opt(optsys);
    opt.setConvergenceTolerance(get_optimization_convergence_tolerance());
    opt.useNumericalGradient(true);

    // Optimize!
    // =========
    opt.optimize(results);

    // Return the solution to the optimization.
    // ========================================
    return tool.createParameterValueSet(results);
}

PSimDynamicOptimizationSolver::OptimizerSystem::OptimizerSystem(
        const PSimTool& tool)
: SimTK::OptimizerSystem(tool.numOptimizerParameters()), m_pstool(tool)
{
}

int PSimDynamicOptimizationSolver::OptimizerSystem::objectiveFunc(
        const SimTK::Vector& parameters, bool new_parameters, SimTK::Real& f)
    const
{
    // Convert parameters to a meaningful form.
    // ========================================
    const PSimParameterValueSet pvalset(
            m_pstool.createParameterValueSet(parameters));

    // Simulate the system and evaluate the goals.
    f = m_pstool.simulate(pvalset);

    return 0;
}
