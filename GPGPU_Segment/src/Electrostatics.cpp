/*
 * Copyright (C) 2010 - Alexandru Gagniuc - <mr.nuke.me@gmail.com>
 * This file is part of ElectroMag.
 *
 * ElectroMag is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ElectroMag is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with ElectroMag.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "CL_Electrostatics.hpp"
#include <iostream>

using electro::pointCharge;
using std::cout;
using std::endl;

void TestCL(Vector3<Array<float> > &fieldLines,
	    Array<pointCharge<float> > &pointCharges, size_t n,
	    float resolution, perfPacket &perfData, bool useCurvature,
	    const char *preferred_platform_name = "")
{
	CLElectrosFunctor<float>::BindDataParams dataParams = {
		&fieldLines, &pointCharges, n,
		resolution,  perfData,	    useCurvature
	};

	if (preferred_platform_name)
		CLtest.SetPreferredPlatform(preferred_platform_name);

	cout << "TestCL: Binding data" << endl;
	CLtest.BindData((void *)&dataParams);
	cout << "TestCL: Starting run" << endl;
	CLtest.Run();
	cout << "TestCL: done" << endl;
}
