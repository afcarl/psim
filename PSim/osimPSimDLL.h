#ifndef osimPSimDLL_H_
#define osimPSimDLL_H_
/* -------------------------------------------------------------------------- *
 *                         OpenSim:  osimPSimDLL.h                            *
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


// IMPORT AND EXPORT
// UNIX
#ifndef WIN32

	#define OSIMPSIM_API


// WINDOWS
#else

	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
	#ifdef OSIMPSIM_EXPORTS
		#define OSIMPSIM_API __declspec(dllexport)
	#else
		#define OSIMPSIM_API __declspec(dllimport)
	#endif

#endif



#endif // osimPSimDLL_H_