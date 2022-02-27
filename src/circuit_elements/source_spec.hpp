#ifndef __SOURCE_SPEC_HPP
#define __SOURCE_SPEC_HPP

#include <vector>
#include <complex>
#include "base_types.hpp"
#include "simulator_types.hpp"

//! The transient specification of ICS and IVS (AC and TRAN_SPEC).
class source_spec
{
    public:
        /*!
            @brief    Default constructor.
        */
		source_spec()
		{
			_type = CONSTANT_SOURCE;
			_ac_val = std::complex<double>(1, 0);
		}

		/*!
			@brief   Returns the AC value of this source.
			@return	 The AC value.
		*/
		std::complex<double> getACVal(void) { return _ac_val; }

		/*!
			@brief  Set the complex AC value with the inputs
			being in polar form.
			@param	ac_mag		The magnitude.
			@param	ac_phase	The phase.
		*/
		void setACVal(double ac_mag, double ac_phase) { _ac_val = std::polar(ac_mag, ac_phase); }

		/*!
			@brief      Returns the type of source (for transient analysis).
			@return 	The type
		*/
		tran_source_t getType(void) { return _type; }

		/*!
			@brief  Sets the type of source (for transient analysis).
			@param 	type	The type
		*/
		void setType(tran_source_t type) { _type = type; }

		/*!
			@brief  Returns the transient values for transient type source.
			For PWL sources it returns the current/voltage values, while
			in all sources, the parameters.
			@return 	The values vector
		*/
		std::vector<double> &getTranVals(void) { return _tran_vals; }

		/*!
			@brief  Returns the transient time values for PWL source.
			@return 	The values vector
		*/
		std::vector<double> &getTranTimes(void) { return _tran_time; }

    private:
        /* AC analysis */
        std::complex<double> _ac_val;       //!< The AC value of the source.

        /* TRAN analysis components */
        std::vector<double> _tran_vals;     //!< The parameters of the transient source (PWL the values vector).
        std::vector<double> _tran_time;     //!< The time values vector of PWL sources.
        tran_source_t _type;                //!< The type of source.
};

#endif // __SOURCE_SPEC_HPP //
