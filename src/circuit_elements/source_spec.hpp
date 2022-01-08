#ifndef __SOURCE_SPEC_HPP
#define __SOURCE_SPEC_HPP

#include <vector>
#include <complex>
#include "base_types.hpp"
#include "simulator_types.hpp"

class source_spec
{
    public:
        /*!
            @brief    Default constructor.
        */
		source_spec()
		{
			_type = CONSTANT_SOURCE;
			_ac_val = 0;
		}

		/*!
			@brief   Returns the AC value of this source.
			@return	 The AC value.
		*/
		std::complex<double> getACVal(void)
		{
			return _ac_val;
		}

		/*!
			@brief  Set the complex AC value with the inputs
			being in polar form.
			@param	ac_mag		The magnitude.
			@param	ac_phase	The phase.
		*/
		void setACVal(double ac_mag, double ac_phase)
		{
			_ac_val = std::polar(ac_mag, ac_phase);
		}

		/*!
			@brief      Returns the type of source (for transient analysis).
			@return 	The type
		*/
		tran_source_t getType(void)
		{
			return this->_type;
		}

		/*!
			@brief  Sets the type of source (for transient analysis).
			@param 	type	The type
		*/
		void setType(tran_source_t type)
		{
			this->_type = type;
		}

		/*!
			@brief  Returns the transient values for transient type source.
			For PWL sources it returns the current/voltage values, while
			in all other sources the parameters.
			@return 	The values vector
		*/
		std::vector<double> &getTranVals(void)
		{
			return this->_tran_vals;
		}

		/*!
			@brief  Returns the transient time values for transient type source.
			This is non-empy only for PWL sources, their time values vector.
			@return 	The values vector
		*/
		std::vector<double> &getTranTimes(void)
		{
			return this->_tran_time;
		}

    private:
        /* AC analysis */
        std::complex<double> _ac_val;

        /* TRAN analysis components */
        std::vector<double> _tran_vals;
        std::vector<double> _tran_time;
        tran_source_t _type;
};

#endif // __SOURCE_SPECS_HPP //
