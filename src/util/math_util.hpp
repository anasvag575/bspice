#ifndef __MATH_UTIL_H
#define __MATH_UTIL_H

#include <type_traits>
#include <vector>
#include <cmath>

/*!
	@brief      Templated routine that fills a vector with values in the interval of
    [start, end] using step. The vector can only contain integral types and uses this
    formula for the fill-in: tvec(i) = start + i * step
	@param      vec    	The vector to be filled.
	@param      start  	Start of the interval
	@param      end		End of the interval
	@param		step	The increment.
*/
template< typename T>
void StepVecGen(std::vector<T> &vec, T start, T end, T step)
{
	static_assert(std::is_arithmetic<T>::value, "Input arguments have to be numeric types");

	/* Avoid infinite loops */
	if(step == 0) return;

	/* Incrementally add */
	while(start <= end)
	{
		vec.push_back(start);
		start += step;
	}
}

/*!
	@brief      Templated routine that fills a vector with values in the interval of
    [start, end] with the exact number being points. The generation is done in linear fashion
    hence the lin in the function name. It uses the same principle as the method linspace() in Matlab/Python.
	@param      vec    	The vector to be filled.
	@param      start  	Start of the interval
	@param      end		End of the interval
	@param		point	The number of points to generate.
*/
template< typename T>
void linspaceVecGen(std::vector<T> &vec, T start, T end, size_t points)
{
	static_assert(std::is_arithmetic<T>::value, "Input arguments have to be numeric types");

    if (points <= 1)
    {
    	vec.push_back(start);
	}
	else
	{
		T delta = abs(end - start) / (points);
		for (size_t i = 0; i < points; ++i) vec.push_back(start + delta * i);
	}
}

/*!
	@brief      Templated routine that fills a vector in a logarithmic scale. In the interval [a, b]
	for each decade that exists, a linear interval is generated of number points.
	It used the same principle as logspace() but merges concepts from linspace(), from Python/Matlab.
	@param      vec    	The vector to be filled.
	@param      start  	Start of the interval
	@param      end		End of the interval
	@param		point	The number of points to generate.
*/
template<typename T>
void logspaceVecGen(std::vector<T> &vec, T start, T end, size_t points)
{
	static_assert(std::is_arithmetic<T>::value, "Input arguments have to be numeric types");

	/* Firstly set a starting point for the initial decade */
	if(start <= 0.0) start = 1;
	if(end <= 0.0) end = 1;

	size_t dec_start = static_cast<size_t>(floor(log10(start)));
	size_t dec_end = static_cast<size_t>(ceil(log10(end)));

	for(; dec_start < dec_end; dec_start++)
	{
		T lin_start = pow(10, dec_start);
		T lin_end = pow(10, dec_start + 1);

		linspaceVecGen(vec, lin_start, lin_end, points);
	}
}

/*!
	@brief      Templated routine that linearly interpolates a value x (itrp) given
	a set of x_val, y_val vectors. The result being the inerpolated value y. This method
	can also extrapolate results.
	@param      vec_x   The vector with the x values.
	@param      vec_y   The vector with the x values.
	@param      itrp	The x value to interpolate.
	@return		The interpolated y value.
*/
template<typename T>
T linearInterpolation(std::vector<T> &vec_x, std::vector<T> &vec_y, T itrp)
{
	static_assert(std::is_arithmetic<T>::value, "Input arguments have to be numeric types");

	size_t i = 0;

	/* In case we have a vector size of 1 */
	if(vec_x.size() == 1) return vec_y[0];

	/* Linear search */
	while(i < vec_x.size())
	{
		if(vec_x[i] > itrp) break;

		i++;
	}

	/* These are the boundary conditions - Check */
	if(i == vec_x.size()) i -= 1;
	else if(i == 0) i = 1;

	/* Interpolate between the 2 points */
	T x0 = vec_x[i - 1], x1 = vec_x[i];
	T y0 = vec_y[i - 1], y1 = vec_y[i];

	return y0 + ((y1 - y0) / (x1 - x0)) * (itrp - x0);
}

#endif // __MATH_UTIL_H //
