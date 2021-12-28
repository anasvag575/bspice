#include "syntax_parser.hpp"

/*!
	@brief      Function that tokenizes the input line and converts, from the spice file.
	The tokens are returned in uppercase only, since SPICE format is not case sensitive.
	Returns, the tokens in the provided argument and whether the vector
	is empty or not (with valid tokens)
	@param      tokens    		  The line to be tokenized.
	@param      tokens    		  The tokens that form the current spice element/card.
	@return     True in case the vector is not empty.
*/
bool syntax_parser::tokenizer(std::string &line, std::vector<std::string> &tokens)
{
	// TODO - Choose the version either strtok or stdregex

    // STRTOK version //
	std::string linecopy(line);
	char *token_tmp = linecopy.data();
	char *tmp = NULL;
	tmp = strtok(token_tmp, _delimiters);

	/* Clear the token vector from previous lines */
	tokens.clear();

	if(tmp) tokens.push_back(tmp);

	while(1)
	{
		tmp = strtok(NULL, _delimiters);
		if(tmp) tokens.push_back(tmp);
		else break;
	}

    if(tokens.empty()) return false;

	// REGEX version //
//    std::sregex_token_iterator first{line.begin(), line.end(), _delimiters, -1}, last;//the '-1' is what makes the regex split (-1 := what was not matched)
//    tokens = {first, last};
//
//    /* Empty line or line with 1 empty string */
//    if(tokens.empty() || (tokens.size() == 1 && !tokens[0].size())) return false;

    /* Convert the strings to uppercase characters */
    for (auto& t : tokens) std::transform(t.begin(), t.end(), t.begin(), ::toupper);

    return true;
}

/*!
	@brief      Function verifies a basic element of this type (Resistors, Coils, Capacitors):
						#####<Element>  <V+>  <V->  <Value>#####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    		  The tokens that form the element.
	@param      converted_val     Value at the end of the element.
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidTwoNodeElement(std::vector<std::string> &tokens, double &converted_val)
{
    if(tokens.size() != 4) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidSetValue(tokens[3], converted_val);

    return format;
}

/*!
	@brief      Function verifies a basic element of this type (Voltage controlled sources):
						#####<Element>  <V+>  <V->  <Vex+>  <Vex->  <Value>#####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    		  The tokens that form the element.
	@param      converted_val     Value at the end of the element.
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidFourNodeElement(std::vector<std::string> &tokens, double &converted_val)
{
    if(tokens.size() != 6) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidNode(tokens[3]) &&
                  IsValidNode(tokens[4]) && IsValidSetValue(tokens[5], converted_val);

    return format;
}

/*!
	@brief      Function verifies a basic element of this type (Current controlled sources):
						#####<Element>  <V+>  <V->  <Element> <Value>#####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    		  The tokens that form the element.
	@param      converted_val     Value at the end of the element.
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidCurrentControlElement(std::vector<std::string> &tokens, double &converted_val)
{
    if(tokens.size() != 5) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidName(tokens[3]) &&
                  IsValidSetValue(tokens[4], converted_val);

    return format;
}

/*!
	@brief      Function verifies a basic element of this type (ICS/IVS):
		#####<Element>  <V+>  <V->  <Value>  {AC <mag> <phase>} {TRAN_SPEC}#####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument and for any of the optional arguments.

	@param      tokens    		  The tokens that form the element.
	@param      converted_val     Value of the source element (DC state value).
	@param      ac_mag     		  Optional - The magnitude of the source in case of AC analysis.
	@param      ac_phase     	  Optional - The phase of the source in case of AC analysis
	@param		vvals			  Optional - The values for TRAN_SPEC (for PWL the y_vals)
	@param		tvals			  Optional - The x_vals (time) values for PWL TRAN_SPEC
	@param		source			  The type of source
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidSource(std::vector<std::string> &tokens, double &converted_val, double &ac_mag, double &ac_phase,
								  std::vector<double> &vvals, std::vector<double> &tvals, tran_source_t source)
{
    /* First verify that it has the minimum requirements */
    if(tokens.size() <= 3) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidSetValue(tokens[3], converted_val);

    size_t tokens_left = tokens.size() - 4;
    bool ac_found = false, tran_found = false;
    size_t idx = 4;
    source = CONSTANT;

    while(tokens_left && format)
    {
        if(tokens[idx] == "AC")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 3 || ac_found) return false;

            /* Verify the needed  */
            format = IsValidSetValue(tokens[idx + 1], ac_mag) && IsValidSetValue(tokens[idx + 2], ac_phase);

            /* Found our first source and incrementing our indices */
            ac_found = true;
            tokens_left -= 3;
            idx += 3;
        }
        else if(tokens[idx] == "EXP" || tokens[idx] == "SIN")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 7 || tran_found) return false;

            vvals.resize(6);

            format &= IsValidSetValue(tokens[idx + 1], vvals[0]) &&
                      IsValidSetValue(tokens[idx + 2], vvals[1]) &&
                      IsValidSetValue(tokens[idx + 3], vvals[2]) &&
                      IsValidSetValue(tokens[idx + 4], vvals[3]) &&
                      IsValidSetValue(tokens[idx + 5], vvals[4]) &&
                      IsValidSetValue(tokens[idx + 6], vvals[5]);

            /* Found our first source and incrementing our indices */
            source = (tokens[idx] == "EXP") ? EXP : SINE;
            tran_found = true;
            tokens_left -= 7;
            idx += 7;
        }
        else if(tokens[idx] == "PULSE")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 8 || tran_found) return false;

            vvals.resize(7);

            format &= IsValidSetValue(tokens[idx + 1], vvals[0]) &&
                      IsValidSetValue(tokens[idx + 2], vvals[1]) &&
                      IsValidSetValue(tokens[idx + 3], vvals[2]) &&
                      IsValidSetValue(tokens[idx + 4], vvals[3]) &&
                      IsValidSetValue(tokens[idx + 5], vvals[4]) &&
                      IsValidSetValue(tokens[idx + 6], vvals[5]) &&
                      IsValidSetValue(tokens[idx + 7], vvals[6]);

            /* Found our first source and incrementing our indices */
            source = PULSE;
            tran_found = true;
            tokens_left -= 8;
            idx += 8;
        }
        else if(tokens[idx] == "PWL" && !tran_found)
        {
            /* Increment the tokens */
            tokens_left -= 1;
            idx += 1;

            while(tokens_left >= 2 && IsValidValue(tokens[idx]))
            {
                double val, tpoint;

                format &= IsValidSetValue(tokens[idx], tpoint) && IsValidSetValue(tokens[idx + 1], val);

                if(format)
                {
                    tvals.push_back(tpoint);
                    vvals.push_back(val);
                    tokens_left -= 2;
                    idx += 2;
                }
                else /* Early out */
                {
                    return false;
                }
            }
        }
        else /* Unknown option */
        {
            return false;
        }
    }

    return format;
}

/*!
	@brief      Function verifies the syntax for a transient analysis spice card (.TRAN):
							##### .TRAN  <tstep>  <tstop>  {tstart} #####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    	The tokens that form the element.
	@param      step     	The time step point of the analysis.
	@param      tstop     	The end time of the analysis
	@param      tstart     	Optional - Starting time of the analysis.
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart)
{
    if(tokens.size() != 3 && tokens.size() != 4) return false;

    tstart = 0; /* Default is to set the simulation start point at 0 */

    /* Verify the validity of all nodes */
    bool format = IsValidSetValue(tokens[1], step) &&
                  IsValidSetValue(tokens[2], tstop);

    /* In case we have start time */
    if(tokens.size() == 4) format &= IsValidSetValue(tokens[3], tstart);

    return format;
}

/*!
	@brief      Function verifies the syntaxes for a direct analysis spice card (.DC):
					##### .DC  DEC/LIN  <Element>  <stop>  <start>  <step> #####
					##### .DC  LOG  <Element>  <stop>  <start>  <points> #####
	Along with this, it returns the appropriate values for each token.

	@param      tokens    	The tokens that form the element.
	@param      points     	Either the step (DEC/LIN scale) or the points per decade (LOG).
	@param      stop     	The end voltage/current of the analysis.
	@param      start     	The start voltage/current of the analysis.
	@param		scale		The scale of the analysis (DEC or LOG)
	@param		source		The element name, of the source under analysis (ICS or IVS)
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidDCCard(std::vector<std::string> &tokens, double &points, double &stop, double &start, as_scale_t &scale, std::string &source)
{
    if(tokens.size() != 6) return false;

    bool format;

    /* Check type of scale */
    if(tokens[1] == "DEC" || tokens[1] == "LIN")
    {
    	scale = DEC_SCALE;
    	format = IsValidSetValue(tokens[5], points);
    }
    else if (tokens[1] == "LOG")
    {
    	scale = LOG_SCALE;
        format = IsValidSetValueInt(tokens[5], points);
    }
    else
    {
    	return false;
    }

    /* Copy source name */
    source = tokens[2];

    /* Verify the validity of all names and valeues */
    format &= IsValidName(tokens[2]) && IsValidSetValue(tokens[3], start) &&
			  IsValidSetValue(tokens[4], stop);

    return format;
}

/*!
	@brief      Function verifies the syntaxes for a frequency analysis spice card (.AC):
					##### .AC  DEC/LIN  <step>  <stop>  <start> #####
					##### .AC  LOG  <points>  <stop>  <start>  #####
	Along with this, it returns the appropriate values for each token.

	@param      tokens    	The tokens that form the element.
	@param      points     	Either the step (DEC/LIN scale) or the points per decade (LOG).
	@param      fstop     	The end voltage/current of the analysis.
	@param      fstart     	The start voltage/current of the analysis.
	@param		scale		The scale of the analysis (DEC or LOG)
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidACCard(std::vector<std::string> &tokens, double &points, double &fstop, double &fstart, as_scale_t &scale)
{
    if(tokens.size() != 5) return false;

    /* Check type of scale */
    if(tokens[1] == "DEC" || tokens[1] == "LIN") scale = DEC_SCALE;
    else if (tokens[1] == "LOG") scale = LOG_SCALE;
    else return false;

    /* Verify the validity of all nodes */
    bool format = IsValidSetValueInt(tokens[2], points) && IsValidSetValue(tokens[3], fstart) &&
                  IsValidSetValue(tokens[4], fstop);

    return format;
}

/*!
	@brief      Function verifies the syntaxes for a plot/print spice card (.PLOT):
					##### .PLOT  V(nodename1) I(Vsourcename1) ... V(nodenameN) #####
	Along with this, it returns the tokens (names of the nodes or sources).

	@param      tokens    		The tokens that form the element.
	@param      plot_nodes		The nodes to be plotted
	@param      plot_sources 	The sources to be plotted
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::isValidSetPLOTCard(std::vector<std::string> &tokens, std::vector<std::string> &plot_nodes, std::vector<std::string> &plot_sources)
{
	size_t tmp_size = tokens.size() - 1;
	size_t idx = 1;

	/* Since tokenization by the initial breakdown of the tokens, eliminates
	 * parentheses we do not have to resplit the tokens based on a different pattern/spec.
	 * The tokens are already in the form of {<V> nodename} or {<I> Vsourcename} */

	/* Size has to be an odd number since we have pairs */
	if(tmp_size % 2) return false;

	/* For each pair insert the element/node name */
	while(tmp_size)
	{
		/* No need to check if they are valid names
		 * This is taken care from the caller
		 */
		if(tokens[idx] == "V")
		{
			plot_nodes.push_back(tokens[idx + 1]);
		}
		else if(tokens[idx] == "I")
		{
			plot_sources.push_back(tokens[idx + 1]);
		}
		else
		{
			return false;
		}

		idx += 2;
		tmp_size -= 2;
	}

	return true;
}

/*!
	@brief      Internal function verifies that verifies the validity of a <NodeType>
	token and returns the result of the appraisal.
	@param      node    	The node's name
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::IsValidNode(const std::string &node)
{
    return std::regex_match(node, _alphanumeric_with_underscores);
}

/*!
	@brief      Internal function verifies that verifies the validity of an <ElementType>
	token and returns the result of the appraisal.
	@param      node    	The element's name
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::IsValidName(const std::string &node)
{
    return std::regex_match(node, _alphanumeric_with_underscores);
}

/*!
	@brief      Internal function verifies that verifies a floating point number
	and returns the result of the appraisal along with the said number.
	@param      node    	The element's name
	@param		val			The floating point number
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::IsValidSetValue(std::string &node, double &val)
{
    if(std::regex_match(node, _decimal_number))
    {
        // TODO - Case for strtod or modify string too
//                if(node[node.size() - 1])
        val = stod(node);

        return true;
    }

    return false;
}

/*!
	@brief      Internal function verifies that verifies a intger number
	and returns the result of the appraisal along with the said number.
	@param      node    	The element's name
	@param		val			The floating point number
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::IsValidSetValueInt(std::string &node, double &val)
{
    if(std::regex_match(node, _integer_number))
    {
        // TODO - Case for strtod or modify string too
//                if(node[node.size() - 1])
        val = stod(node);

        return true;
    }

    return false;
}

/*!
	@brief      Internal function verifies that verifies a floating point number
	and returns the result of the appraisal.
	@param      node    	The element's name
	@return     Valid(true) syntax or not(false)
*/
bool syntax_parser::IsValidValue(std::string &node)
{
    if(std::regex_match(node, _decimal_number))
    {
        // TODO - Case for strtod or modify string too
//                if(node[node.size() - 1])
//        val = stod(node);

        return true;
    }

    return false;
}
