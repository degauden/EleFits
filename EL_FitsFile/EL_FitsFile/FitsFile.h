/**
 * @file EL_FitsFile/FitsFile.h
 * @date 07/23/19
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

#ifndef _EL_FITSFILE_FITSFILE_H
#define _EL_FITSFILE_FITSFILE_H

#include <cfitsio/fitsio.h>
#include <string>

namespace Euclid {
namespace FitsIO {

/**
 * @brief Fits file handler.
 */
class FitsFile {

public:

	enum class Permission {
		READ,
		EDIT,
		CREATE,
		OVERWRITE
	};

	/**
	 * @brief Create a new Fits file handler with given filename and permission.
	 */
	FitsFile(std::string filename, Permission permission);

	/**
	 * @brief Destroy the FitsFile handler.
	 */
	~FitsFile();

	/**
	 * @brief Open a Fits file.
	 */
	void open(std::string filename, Permission permission);

	/**
	 * @brief Close the Fits file.
	 */
	void close();

	/**
	 * @brief Close and delete the Fits file.
	 */
	void close_and_delete();

private:

	fitsfile* m_fptr;

};

}
}

#endif
