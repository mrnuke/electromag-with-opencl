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
#include "SOA_utils.hpp"
#include <assert.h>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;
using std::ofstream;

/*
 * Compare electric fields 'field_a' and 'field_b' and find diverging lines
 *
 * Compare to electric fields that are preusmed to be identical. Looks for
 * linea in 'b' that are close to the line in 'a' but suddenly jump off-course
 */
void compare_electric_fields(Vector3<Array<float> > &field_a,
			     Vector3<Array<float> > &field_b, size_t num_lines,
			     size_t line_len, const char *output_filename)
{
	Vector3<float> a, b, ap, bp;
	ofstream regress;

	/* Okay to use assert in debug-only code */
	assert(field_a.GetSize() == num_lines * line_len);
	assert(field_a.GetSize() == field_b.GetSize());

	regress.open(output_filename);

	cout << " Beginning verfication procedure" << endl;
	for (size_t line = 0; line < num_lines; line++) {
		//
		size_t step = 1;
		do {
			size_t i = step * num_lines + line;
			size_t iLast = (step - 1) * num_lines + line;

			a = field_a[i];
			b = field_b[i];
			ap = field_a[iLast];
			bp = field_b[iLast];

			// Calculate the distance between the a and b point
			float offset3D = vec3Len(vec3(a, b));
			if (offset3D > 0.1f) {
				regress << " good [" << line << "][" << step - 1 << "] x: " << ap.x << " y: " << ap.y << " z: " << ap.z << endl
					<< " bad  [" << line << "][" << step - 1 << "] x: " << bp.x << " y: " << bp.y << " z: " << bp.z << endl
					<< " 3D offset: " << vec3Len(vec3(ap, bp)) << endl;
				regress << " good [" << line << "][" << step << "] x: " << a.x << " y: " << a.y << " z: " << a.z << endl
					<< " bad  [" << line << "][" << step << "] x: " << b.x << " y: " << b.y << " z: " << b.z << endl
					<< " 3D offset: " << offset3D << endl
					<< endl;
				// If a leap is found; skip the current line
				break;
			}
		} while (++step < line_len);

		// Small anti-boredom indicator
		if ((line % 256) == 0)
			cout << " " << line / num_lines * 100 << " % complete"
			     << endl;
	}

	regress.close();
	cout << " Verification complete" << endl;
}
