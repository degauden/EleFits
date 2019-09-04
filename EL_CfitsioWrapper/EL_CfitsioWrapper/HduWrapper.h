/**
 * @file EL_CfitsioWrapper/HduWrapper.h
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

#ifndef _EL_CFITSIOWRAPPER_HDUWRAPPER_H
#define _EL_CFITSIOWRAPPER_HDUWRAPPER_H

#include <cfitsio/fitsio.h>
#include <string>

#include "EL_CfitsioWrapper/BintableWrapper.h"
#include "EL_CfitsioWrapper/CfitsioUtils.h"
#include "EL_CfitsioWrapper/ImageWrapper.h"
#include "EL_CfitsioWrapper/TypeWrapper.h"

namespace Euclid {
namespace Cfitsio {

/**
 * @brief HDU-related functions.
 * 
 * An HDU can be of three Types (ASCII tables are not supported):
 * * METADATA (image HDU with empty data, e.g. Primary of MultiExtFile)
 * * IMAGE
 * * TABLE
 * 
 * Getter functions generally apply to the current HDU.
 * Functions to go to an HDU return false if target HDU is already current HDU.
 * Functions to create an HDU append it at the end of the file.
 */
namespace Hdu {

/**
 * @brief HDU type (ASCII tables not supported).
 */
enum class Type {
	IMAGE, ///< Image HDU
	BINTABLE ///< Binary table HDU
};

/**
 * @brief Read the number of HDUs in a Fits file.
 */
std::size_t count(fitsfile *fptr);

/**
 * @brief Get the index of the current HDU.
 */
std::size_t current_index(fitsfile *fptr);

/**
 * @brief Get the name of the current HDU.
 */
std::string current_name(fitsfile *fptr);

/**
 * @brief Get the Type of the current HDU.
 */
Type current_type(fitsfile *fptr);

/**
 * @brief Check whether current HDU is the Primary HDU.
 */
bool current_is_primary(fitsfile *fptr);

/**
 * @brief Go to an HDU specified by its index.
 */
bool goto_index(fitsfile *fptr, std::size_t index);

/**
 * @brief Go to an HDU specified by its name.
 */
bool goto_name(fitsfile *fptr, std::string name);

/**
 * @brief Go to an HDU specified by incrementing the index by a given amount.
 */
bool goto_next(fitsfile *fptr, std::size_t step=1);

/**
 * @brief Go to the Primary HDU.
 */
bool goto_primary(fitsfile *fptr);

/**
 * @brief Initialize the Primary HDU if not done.
 */
bool init_primary(fitsfile *fptr);

/**
 * @brief Write or update HDU name.
 */
bool update_name(fitsfile* fptr, std::string name);

/**
 * @brief Create a HDU of Type METADATA.
 */
void create_metadata_extension(fitsfile *fptr, std::string name);

/**
 * @brief Create a new Image HDU with given name, pixel type and shape.
 */
template<typename T, std::size_t n=2>
void create_image_extension(fitsfile *fptr, std::string name, const Image::pos_type<n>& shape);

/**
 * @brief Write a Raster in a new Image HDU.
 */
template<typename T, std::size_t n=2>
void create_image_extension(fitsfile *fptr, std::string name, const Image::Raster<T, n>& raster);

/**
 * @brief Create a new Bintable HDU with given name and column infos.
 */
template<typename... Ts>
void create_bintable_extension(fitsfile *fptr, std::string name, const Bintable::column_info<Ts>&... header);

/**
 * @brief Write a Table in a new Bintable HDU.
 */
template<typename... Ts>
void create_bintable_extension(fitsfile *fptr, std::string name, const Bintable::Column<Ts>&... table);


/////////////////////
// IMPLEMENTATION //
///////////////////


template<typename T, std::size_t n>
void create_image_extension(fitsfile *fptr, std::string name, const Image::pos_type<n>& shape) {
	may_throw_readonly_error(fptr);
	int status = 0;
	auto nonconst_shape = shape; //TODO const-correctness issue?
	fits_create_img(fptr, TypeCode<T>::bitpix(), n, &nonconst_shape[0], &status);
	may_throw_cfitsio_error(status);
	update_name(fptr, name);
}

template<typename T, std::size_t n>
void create_image_extension(fitsfile *fptr, std::string name, const Image::Raster<T, n>& raster) {
	may_throw_readonly_error(fptr);
	create_image_extension<T, n>(fptr, name, raster.shape);
	Image::write_raster<T, n>(fptr, raster);
}

template<typename... Ts>
void create_bintable_extension(fitsfile* fptr, std::string name, const Bintable::column_info<Ts>&... header) {
	constexpr std::size_t count = sizeof...(Ts);
	c_str_array col_name { std::get<0>(header) ... };
	c_str_array col_format { TypeCode<Ts>::bintable_format(std::get<1>(header)) ... };
	c_str_array col_unit { std::get<2>(header) ... };
	int status = 0;
	fits_create_tbl(fptr, BINARY_TBL, 0, count,
			col_name.data(), col_format.data(), col_unit.data(),
			name.c_str(), &status);
	may_throw_cfitsio_error(status);
}

template<typename... Ts>
void create_bintable_extension(fitsfile* fptr, std::string name, const Bintable::Column<Ts>&... table) {
	constexpr std::size_t count = sizeof...(Ts);
	c_str_array col_name { table.name ... };
	c_str_array col_format { TypeCode<Ts>::bintable_format(table.repeat) ... };
	c_str_array col_unit { table.unit ... };
	int status = 0;
	fits_create_tbl(fptr, BINARY_TBL, 0, count,
			col_name.data(), col_format.data(), col_unit.data(),
			name.c_str(), &status);
	may_throw_cfitsio_error(status);
	using mock_unpack = int[];
    (void)mock_unpack {(Bintable::write_column(fptr, table), 0)...};
}

/// @cond INTERNAL
template<typename T>
void create_bintable_extension(fitsfile* fptr, std::string name, const Bintable::Column<T>& column) {
	constexpr std::size_t count = 1;
	std::string col_name = column.name;
	char* c_name = &col_name[0];
	std::string col_format = TypeCode<T>::bintable_format(column.repeat);
	char* c_format = &col_format[0];
	std::string col_unit = column.unit;
	char* c_unit = &col_unit[0];
	int status = 0;
	fits_create_tbl(fptr, BINARY_TBL, 0, count,
			&c_name, &c_format, &c_unit,
			name.c_str(), &status);
	may_throw_cfitsio_error(status);
	Bintable::write_column(fptr, column);
}
/// @endcond

}
}
}

#endif
