#include "syntax_parser.hpp"
#include <iostream> /* TOD0 - Debug */

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

/************** SPICE ELEMENTS **************/

/*!
    @brief      Routine parses the token stream and checks the input maps where it:
                    - Verifies correct grammar and syntax of the tokens.
                    - Verifies the uniqueness of the element.
                    - Checks if new nodes are added to the circuit.
    The values then are loaded in this element, therefore this routine serving as the main initializer
    of this type of element. <2-node basic> is an element of this type (ETYPE = R/L/C):
                            ##### ETYPE<name> <V+> <V-> <Value> #####
    @param      tokens    The tokens that form the coil element.
    @param      node2_device   Element reference.
    @param      nodes     	Map that contains all the nodes in the circuit along with their unique nodeNum.
    @param      elements  	Map that contains all the unique elements along with their ID in the circuit.
    @param      id   		Unique ID of this element.
    @param		complete	Flag if node device to be parsed is node2(true) or node2-xxx(false)
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e syntax_parser::Parse2NodeDevice(std::vector<std::string> &tokens,
											   node2_device &element,
											   hashmap_str_t &elements,
											   hashmap_str_t &nodes,
											   size_t device_id,
											   bool complete)
{
	double val;
	auto &node_names = element.getNodeNames();
	auto &node_IDs = element.getNodeIDs();

	/* Check the size of the tokens at hand.
	 * Complete devices need exactly 4, while extended at least 4 */
	bool legal_tokens = complete ? (tokens.size() == 4) : (tokens.size() >= 4);

    /* Check correct syntax */
    if(!legal_tokens || !isValidTwoNodeElement(tokens, val)) return FAIL_PARSER_INVALID_FORMAT;

    /* Check uniqueness  */
    if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
    elements[tokens[0]] = device_id;
    element.setName(tokens[0]);

    /* No short circuits for any elements allowed */
    if(tokens[1] == tokens[2]) return FAIL_PARSER_SHORTED_ELEMENT;

	for(int i = 0; i < 2; i++)
	{
		/* Ground node - Do not insert in map (-1) */
		if(tokens[i + 1] == "0")
		{
			node_names[i] = "0";
			node_IDs[i] = -1;
			continue;
		}

		/* Normal node */
		auto it = nodes.find(tokens[i + 1]);

		if(it != nodes.end())
		{
			node_names[i] = tokens[i + 1];
			node_IDs[i] = it->second;
		}
		else /* First time encountering this node */
		{
			node_names[i] = tokens[i + 1];
			node_IDs[i] = nodes.size();
			nodes[node_names[i]] = node_IDs[i]; /* Also insert in map */
		}
	}

    /* Set the value for the element */
    element.setVal(val);

    return RETURN_SUCCESS;
}

/*!
    @brief      Routine parses the token stream:
                    - Verifies correct grammar and syntax of the tokens.
                    - Checks for duplicate AC or TRAN_SPEC.
    The values then are loaded in this element, therefore this routine serving as the main initializer
    of this type of element. A source_spec belongs to an element of this type
    (2-node extend-2 => ETYPE = I/V):
                 ##### ETYPE<name> <V+> <V-> <Value> {AC <mag> <phase>} {TRAN_SPEC} #####
    @param      tokens    		The tokens that form the source_spec.
    @param      source_spec    Source spec reference.
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e syntax_parser::ParseSourceSpec(std::vector<std::string> &tokens, source_spec &spec)
{
    /* Verify */
    bool format = true, ac_found = false, tran_found = false;
    size_t tokens_left = tokens.size() - 4;
    size_t idx = 4;

    auto &tvals = spec.getTranTimes();
    auto &vvals = spec.getTranVals();

    while(tokens_left && format)
    {
        if(tokens[idx] == "AC")
        {
        	double ac_mag, ac_phase;

            /* Check that there are enough tokens */
            if(tokens_left < 3 || ac_found) return FAIL_PARSER_SOURCE_SPEC_ARGS;

            /* Verify the needed  */
            format = IsValidSetValue(tokens[idx + 1], ac_mag) && IsValidSetValue(tokens[idx + 2], ac_phase);

            /* Set the AC value */
            spec.setACVal(ac_mag, ac_phase);

            /* Found our first source and incrementing our indices */
            ac_found = true;
            tokens_left -= 3;
            idx += 3;
        }
        else if(tokens[idx] == "EXP" || tokens[idx] == "SIN")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 7 || tran_found) return FAIL_PARSER_SOURCE_SPEC_ARGS;

            vvals.resize(6);

            for(int i = 0; i < 6; i++)
            {
            	format = IsValidSetValue(tokens[idx + i + 1], vvals[i]);
            	if(!format) break;
            }

//            /* TODO - Debug */
//            std::cout << "Found EXP-SIN spec:" << std::endl;
//            for(auto it: vvals) std::cout << "\t" << it << std::endl;

            /* Found our first source and incrementing our indices */
            spec.setType((tokens[idx] == "EXP") ? EXP_SOURCE : SINE_SOURCE);
            tran_found = true;
            tokens_left -= 7;
            idx += 7;
        }
        else if(tokens[idx] == "PULSE")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 8 || tran_found) return FAIL_PARSER_SOURCE_SPEC_ARGS;

            vvals.resize(7);

            for(int i = 0; i < 7; i++)
            {
				format = IsValidSetValue(tokens[idx + i + 1], vvals[i]);
            	if(!format) break;
            }

//            /* TODO - Debug */
//            std::cout << "Found PULSE spec:" << std::endl;
//            for(auto it: vvals) std::cout << "\t" << it << std::endl;
//
//            std::cout << "Tokens: " << std::endl;
//            for(auto it: tokens) std::cout << "\t" << it << std::endl;

            /* Found our first source and incrementing our indices */
            spec.setType(PULSE_SOURCE);
            tran_found = true;
            tokens_left -= 8;
            idx += 8;
        }
        else if(tokens[idx] == "PWL" && !tran_found)
        {
            /* Increment the tokens */
        	spec.setType(PWL_SOURCE);
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
            }

//            /* TODO - Debug */
//            std::cout << "Found PWL spec:" << std::endl;
//            for(int i = 0; i < tvals.size(); i++) std::cout << "\t" << tvals[i] << "\t" << vvals[i]<< std::endl;
        }
        else /* Unknown option */
        {
            return FAIL_PARSER_SOURCE_SPEC_ARGS;
        }
    }

    return (format) ? RETURN_SUCCESS : FAIL_PARSER_SOURCE_SPEC_ARGS;
}

/************** SPICE CARDS **************/

/*!
	@brief      Function verifies the syntax for a transient analysis spice card (.TRAN):
							##### .TRAN  <tstep>  <tstop>  {tstart} #####
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    	The tokens that form the element.
	@param      step     	The time step point of the analysis.
	@param      tstop     	The end time of the analysis
	@param      tstart     	Optional - Starting time of the analysis.
	@return     The error code in case of error, otherwise RETURN_SUCESS.
*/
return_codes_e syntax_parser::isValidTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart)
{
    if(tokens.size() != 3 && tokens.size() != 4) return FAIL_PARSER_INVALID_FORMAT;

    tstart = 0; /* Default is to set the simulation start point at 0 */

    /* Verify the validity of all nodes */
    bool format = IsValidSetValue(tokens[1], step) &&
                  IsValidSetValue(tokens[2], tstop);

    /* In case we have start time */
    if(tokens.size() == 4) format &= IsValidSetValue(tokens[3], tstart);

    if(!format) return FAIL_PARSER_INVALID_FORMAT;

	/* Timing checks */
	if((tstop <= tstart) || (step <= 0) || (tstart < 0.0))
		return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
}

/*!
	@brief      Function verifies the syntaxes for a direct analysis spice card (.DC):
					##### .DC  {DEC/LIN } <Element>  <stop>  <start>  <step> #####
					##### .DC  {LOG}  <Element>  <stop>  <start>  <points> #####
	Along with this, it returns the appropriate values for each token.
	@param      tokens    	The tokens that form the element.
	@param      points     	Either the step (DEC/LIN scale) or the points per decade (LOG).
	@param      stop     	The end voltage/current of the analysis.
	@param      start     	The start voltage/current of the analysis.
	@param		scale		The scale of the analysis (DEC or LOG)
	@param		source		The element name, of the source under analysis (ICS or IVS)
	@return     The error code in case of error, otherwise RETURN_SUCESS.
*/
return_codes_e syntax_parser::isValidDCCard(std::vector<std::string> &tokens, double &points, double &stop, double &start, as_scale_t &scale, std::string &source)
{
    if(tokens.size() != 5 && tokens.size() != 6) return FAIL_PARSER_INVALID_FORMAT;

    int index = 1;

    /* Check type of scale - Optional */
    if(tokens.size() == 6)
    {
        if(tokens[1] == "DEC" || tokens[1] == "LIN") scale = DEC_SCALE;
        else if (tokens[1] == "LOG") scale = LOG_SCALE;
        else return FAIL_PARSER_INVALID_FORMAT;

        index++;
    }

    /* Copy source name */
    source = tokens[index];

    /* Verify the validity of all names */
    bool format = IsValidName(tokens[index]) && IsValidSetValue(tokens[index + 1], start) &&
    			  IsValidSetValue(tokens[index + 2], stop);

    /* Steps are parsed according to scale */
    if(scale == DEC_SCALE) format &= IsValidSetValue(tokens[index + 3], points);
    else format &= IsValidSetValueInt(tokens[index + 3], points);

    /* Syntactic error */
    if(!format) return FAIL_PARSER_INVALID_FORMAT;

	/* Timing checks */
	if((stop <= start) || (points <= 0)) return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

	/* It has to be a voltage source */
	if(source[0] != 'V' && source[0] == 'I') return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
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
	@return     The error code in case of error, otherwise RETURN_SUCESS.
*/
return_codes_e syntax_parser::isValidACCard(std::vector<std::string> &tokens, double &points, double &fstop, double &fstart, as_scale_t &scale)
{
    if(tokens.size() != 5) return FAIL_PARSER_INVALID_FORMAT;

    /* Check type of scale */
    if(tokens[1] == "DEC" || tokens[1] == "LIN") scale = DEC_SCALE;
    else if (tokens[1] == "LOG") scale = LOG_SCALE;
    else return FAIL_PARSER_INVALID_FORMAT;

    /* Verify the validity of all nodes */
    bool format = IsValidSetValueInt(tokens[2], points) && IsValidSetValue(tokens[3], fstart) &&
                  IsValidSetValue(tokens[4], fstop);

    if(!format) return FAIL_PARSER_INVALID_FORMAT;

	/* Timing checks */
	if((fstop <= fstart) || (points <= 0) || (fstart <= 0.0))
		return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
}

/*!
	@brief      Function verifies the syntaxes for a plot/print spice card (.PLOT):
					##### .PLOT  V(nodename1) I(Vsourcename1) ... V(nodenameN) #####
	Along with this, it returns the tokens (names of the nodes or sources).
	@param      tokens    		The tokens that form the element.
	@param      plot_nodes		The nodes to be plotted
	@param      plot_sources 	The sources to be plotted
	@return     The error code in case of error, otherwise RETURN_SUCESS.
*/
return_codes_e syntax_parser::isValidSetPLOTCard(std::vector<std::string> &tokens, std::vector<std::string> &plot_nodes, std::vector<std::string> &plot_sources)
{
	size_t tmp_size = tokens.size() - 1;
	size_t idx = 1;

	/* Since tokenization by the initial breakdown of the tokens, eliminates
	 * parentheses we do not have to resplit the tokens based on a different pattern/spec.
	 * The tokens are already in the form of {<V> nodename} or {<I> Vsourcename} */

	/* Size has to be an odd number since we have pairs */
	if(tmp_size % 2) return FAIL_PARSER_INVALID_FORMAT;

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
			return FAIL_PARSER_INVALID_FORMAT;
		}

		idx += 2;
		tmp_size -= 2;
	}

	return RETURN_SUCCESS;
}

/************** UTIL **************/

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
