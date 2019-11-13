/**
 * @file src/lib/FitsDataFixture.cpp
 * @date 10/21/19
 * @author user
 *
 * @copyright (C) 2012-2020 Euclid Science Ground Segment
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "EL_FitsData/FitsDataFixture.h"


namespace Euclid {
namespace FitsIO {
namespace Test {

SmallRaster::SmallRaster(long width, long height) :
		FitsIO::Raster<float>({width, height}),
		width(width), height(height) {
	for(int x=0; x<shape[0]; ++x)
		for(int y=0; y<shape[1]; ++y)
			operator[]({x, y}) = 0.1 * y + x;
}

bool SmallRaster::approx(const FitsIO::Raster<float>& other, float tol) const {
	if(other.shape != this->shape)
		return false;
	for(std::size_t i=0; i<this->size(); ++i) {
		const auto o = other.data[i];
		const auto t = this->data[i];
		const auto rel = (o - t) / t;
		if(rel > 0 && rel > tol)
			return false;
		if(rel < 0 && -rel > tol)
			return false;
	}
	return true;
}

SmallTable::SmallTable() :
		extname("MESSIER"),
		nums { 45, 7, 31 },
		radecs { {56.8500, 24.1167}, {268.4667, -34.7928}, {10.6833, 41.2692} },
		names { "Pleiades", "Ptolemy Cluster", "Andromeda Galaxy" },
		dists_mags { { 0.44, 1.6 }, { 0.8, 3.3 }, { 2900., 3.4 } },
		num_col { "ID", 1, "", nums },
		radec_col { "RADEC", 1, "deg", radecs },
		name_col { "NAME", 68, "", names }, //TODO 68?
		dist_mag_col { "DIST_MAG", 2, "kal", dists_mags } {
}

}
}
}
