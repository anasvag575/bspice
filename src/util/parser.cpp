#include "parser.hpp"

/*!
	@brief      Function that tokenizes the input line from the spice file.
	The tokens are returned in uppercase only, since SPICE format is not case sensitive.
	Returns, the tokens in the provided argument and whether the vector is empty or not (with any valid tokens).
	@param      line    		  The line to be tokenized.
	@param      tokens    		  The tokens that form the current spice element/card.
	@return     True in case the vector is not empty, otherwise false.
*/
bool parser::tokenizer(std::string &line, std::vector<std::string> &tokens)
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
    //the '-1' is what makes the regex split (-1 := what was not matched)
//    std::sregex_token_iterator first{line.begin(), line.end(), _delimiters, -1}, last;
//    tokens = {first, last};
//
//    /* Empty line or line with 1 empty string */
//    if(tokens.empty() || (tokens.size() == 1 && !tokens[0].size())) return false;

    /* Convert the strings to uppercase characters */
    for (auto& t : tokens) std::transform(t.begin(), t.end(), t.begin(), ::toupper);

    return true;
}



/*!
    @brief  Routine parses the token stream and checks the input maps where it:
            - Verifies correct grammar and syntax of the tokens.
            - Verifies the uniqueness of the element.
            - Checks if new nodes are added to the circuit.
    The values then are loaded in this element, therefore this routine serves as the main initializer
    of this type of element (2-node-basic and 2-node-extend).\n
    [2-node-basic] is an element of this type  => [R/L/C][name] [V+] [V-] [Value]\n
    [2-node-extend] is an element of this type => [I/V][name] [V+] [V-] [Value] {AC [mag] [phase]} {TRAN_SPEC}\n
    @param      tokens      The tokens that form the element.
    @param      element     Element reference.
    @param      nodes     	Map that contains all the nodes in the circuit along with their unique nodeNum.
    @param      elements  	Map that contains all the unique elements along with their ID in the circuit.
    @param      device_id   Unique ID of this element.
    @param		complete	Flag if device to be parsed is 2-node-basic(true) or 2-node-extend(false)
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parse2NodeDevice(std::vector<std::string> &tokens,
										node2_device &element,
										hashmap_str_t &elements,
										hashmap_str_t &nodes,
										size_t device_id,
										bool complete)
{
	/* Check the size of the tokens at hand.
	 * Complete devices need exactly 4, while extended at least 4 */
	bool legal_tokens = complete ? (tokens.size() == 4) : (tokens.size() >= 4);

    /* Check correct syntax */
    if(!legal_tokens || !isValidTwoNodeElement(tokens)) return FAIL_PARSER_INVALID_FORMAT;

    /* Check uniqueness  */
    if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
    elements[tokens[0]] = device_id;

    /* No short circuits for any elements allowed */
    if(tokens[1] == tokens[2]) return FAIL_PARSER_SHORTED_ELEMENT;

    /* Create the device parameters */
    auto posID = resolveNodeID(nodes, tokens[1]);
    auto negID = resolveNodeID(nodes, tokens[2]);
    auto val = resolveFloatNum(tokens[3]);

    /* Set */
    element.setName(tokens[0]);
    element.setNodeNames(tokens[1], tokens[2]);
    element.setNodeIDs(posID, negID);
    element.setVal(val);

    return RETURN_SUCCESS;
}

/*!
    @brief  Routine parses the token stream and checks the input maps where it:
            - Verifies correct grammar and syntax of the tokens.
            - Verifies the uniqueness of the element.
            - Checks if new nodes are added to the circuit.
    The values then are loaded in this element, therefore this routine serves as the main initializer
    of this type of element (2-node-source).\n
    [2-node-source] is an element of this type => [H/F][name] [V+] [V-] [Vname] [Value]\n
    @param      tokens      The tokens that form the element.
    @param      element     Element reference.
    @param      nodes       Map that contains all the nodes in the circuit along with their unique nodeNum.
    @param      elements    Map that contains all the unique elements along with their ID in the circuit.
    @param      device_id   Unique ID of this element.
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parse2SNodeDevice(std::vector<std::string> &tokens,
                                         node2s_device &element,
                                         hashmap_str_t &elements,
                                         hashmap_str_t &nodes,
                                         size_t device_id)
{
    /* Check correct syntax */
    if(!isValidCurrentControlElement(tokens)) return FAIL_PARSER_INVALID_FORMAT;

    /* Check uniqueness  */
    if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
    elements[tokens[0]] = device_id;

    /* No short circuits for any elements allowed */
    if(tokens[1] == tokens[2]) return FAIL_PARSER_SHORTED_ELEMENT;

    /* Create the device parameters */
    auto posID = resolveNodeID(nodes, tokens[1]);
    auto negID = resolveNodeID(nodes, tokens[2]);
    auto val = resolveFloatNum(tokens[4]);

    /* Set */
    element.setName(tokens[0]);
    element.SetSourceName(tokens[3]);
    element.setNodeNames(tokens[1], tokens[2]);
    element.setNodeIDs(posID, negID);
    element.setVal(val);

    return RETURN_SUCCESS;
}

/*!
    @brief  Routine parses the token stream and checks the input maps where it:
            - Verifies correct grammar and syntax of the tokens.
            - Verifies the uniqueness of the element.
            - Checks if new nodes are added to the circuit.
    The values then are loaded in this element, therefore this routine serving as the main initializer
    of this type of element (4-node-basic).
    [4-node-basic] is an element of this type (ETYPE = E/G) => [E/G][name] [V+] [V-] [Vd+] [Vd-] [Value]
                            #####  #####
    @param      tokens      The tokens that form the coil element.
    @param      element     Element reference.
    @param      nodes       Map that contains all the nodes in the circuit along with their unique nodeNum.
    @param      elements    Map that contains all the unique elements along with their ID in the circuit.
    @param      device_id   Unique ID of this element.
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parse4NodeDevice(std::vector<std::string> &tokens,
                                               node4_device &element,
                                               hashmap_str_t &elements,
                                               hashmap_str_t &nodes,
                                               size_t device_id)
{
    /* Check correct syntax */
    if(!isValidFourNodeElement(tokens)) return FAIL_PARSER_INVALID_FORMAT;

    /* Check uniqueness  */
    if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
    elements[tokens[0]] = device_id;

    /* No short circuits for any elements allowed */
    if(tokens[1] == tokens[2] || tokens[3] == tokens[4]) return FAIL_PARSER_SHORTED_ELEMENT;

    /* Create the device parameters */
    auto posID = resolveNodeID(nodes, tokens[1]);
    auto negID = resolveNodeID(nodes, tokens[2]);
    auto dep_posID = resolveNodeID(nodes, tokens[3]);
    auto dep_negID = resolveNodeID(nodes, tokens[4]);
    auto val = resolveFloatNum(tokens[5]);

    /* Set */
    element.setName(tokens[0]);
    element.setNodeNames(tokens[1], tokens[2]);
    element.setDepNodeNames(tokens[3], tokens[4]);
    element.setNodeIDs(posID, negID);
    element.setDepNodeIDs(dep_posID, dep_negID);
    element.setVal(val);

    return RETURN_SUCCESS;
}

/*!
    @brief  Routine parses the token stream:
            - Verifies correct grammar and syntax of the tokens.
            - Checks for duplicate AC or TRAN_SPEC.
    The values then are loaded in this element, therefore this routine serves as the main initializer
    of this type of element (2-node extend).\n
    A source_spec belongs to an element of [2-node extend] => [I/V][name] [V+] [V-] [Value] {AC [mag] [phase]} {TRAN_SPEC}\n
    @param      tokens      The tokens that form the source_spec.
    @param      spec        Source spec reference.
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parseSourceSpec(std::vector<std::string> &tokens, source_spec &spec)
{
    /* Verify */
    bool format = true, ac_found = false, tran_found = false;
    size_t tokens_left = tokens.size() - 4;
    size_t idx = 4;

    auto &tvals = spec.TranTimes();
    auto &vvals = spec.TranVals();

    /* TODO - More readable and efficient */
    while(tokens_left && format)
    {
        if(tokens[idx] == "AC")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 3 || ac_found) return FAIL_PARSER_SOURCE_SPEC_ARGS_NUM;

            /* Verify the needed  */
            if(!IsValidFpValue(tokens[idx + 1]) || !IsValidFpValue(tokens[idx + 2]))
                return FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT;

            /* Set the values */
            double ac_mag = resolveFloatNum(tokens[idx + 1]);
            double ac_phase = M_PI/180 * resolveFloatNum(tokens[idx + 2]); // Also have to convert to radians
            spec.setACVal(ac_mag, ac_phase);

            /* Found our first source and incrementing our indices */
            ac_found = true;
            tokens_left -= 3;
            idx += 3;
        }
        else if(tokens[idx] == "EXP" || tokens[idx] == "SIN")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 7 || tran_found) return FAIL_PARSER_SOURCE_SPEC_ARGS_NUM;

            for(int i = 0; i < 6; i++)
            {
                if(!IsValidFpValue(tokens[idx + i + 1])) return FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT;
            }

            /* Set */
            vvals.resize(6);
            for(int i = 0; i < 6; i++) vvals[i] = resolveFloatNum(tokens[idx + 1 + i]);

            /* Found our first source and incrementing our indices */
            spec.setType((tokens[idx] == "EXP") ? EXP_SOURCE : SINE_SOURCE);
            tran_found = true;
            tokens_left -= 7;
            idx += 7;
        }
        else if(tokens[idx] == "PULSE")
        {
            /* Check that there are enough tokens */
            if(tokens_left < 8 || tran_found) return FAIL_PARSER_SOURCE_SPEC_ARGS_NUM;

            for(int i = 0; i < 7; i++)
            {
                if(!IsValidFpValue(tokens[idx + i + 1])) return FAIL_PARSER_SOURCE_SPEC_ARGS_FORMAT;
            }

            /* Set */
            vvals.resize(7);
            for(int i = 0; i < 7; i++) vvals[i] = resolveFloatNum(tokens[idx + 1 + i]);

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

            while(tokens_left >= 2 && IsValidFpValue(tokens[idx]))
            {
                if(IsValidFpValue(tokens[idx + 1])) /* Lookahead one more character, if valid insert */
                {
                    tvals.push_back(resolveFloatNum(tokens[idx]));
                    vvals.push_back(resolveFloatNum(tokens[idx + 1]));
                    tokens_left -= 2;
                    idx += 2;
                }
                else
                {
                    break;
                }
            }
        }
        else /* Unknown option */
        {
            return FAIL_PARSER_SOURCE_SPEC_ARGS;
        }
    }

    return RETURN_SUCCESS;
}



/*!
    @brief  Function verifies the syntaxes for a direct analysis spice card (.DC):\n
            => .DC  [DEC/LIN] [Element]  [stop]  [start]  step]\n
            => .DC  LOG  [Element]  [stop]  [start]  [points]\n
    Along with this, it returns the appropriate values for each token.
    @param      tokens      The tokens that form the element.
    @param      points      Either the step (DEC/LIN scale) or the points per decade (LOG).
    @param      stop        The end voltage/current of the analysis.
    @param      start       The start voltage/current of the analysis.
    @param      scale       The scale of the analysis (DEC or LOG)
    @param      source      The element name, of the source under analysis (ICS or IVS)
    @return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parseDCCard(std::vector<std::string> &tokens,
                                   double &points,
                                   double &stop,
                                   double &start,
                                   as_scale_t &scale,
                                   std::string &source)
{
    if(tokens.size() != 5 && tokens.size() != 6) return FAIL_PARSER_INVALID_FORMAT;

    int index = 1;
    scale = DEC_SCALE;

    /* Check type of scale - Optional */
    if(tokens.size() == 6)
    {
        if(tokens[1] == "DEC" || tokens[1] == "LIN") scale = DEC_SCALE;
        else if (tokens[1] == "LOG") scale = LOG_SCALE;
        else return FAIL_PARSER_INVALID_FORMAT;

        index++;
    }

    /* Verify syntax */
    bool format = IsValidName(tokens[index]) && IsValidFpValue(tokens[index + 1]) && IsValidFpValue(tokens[index + 2]);
    format &= (scale == DEC_SCALE) ? IsValidFpValue(tokens[index + 3]):IsValidIntValue(tokens[index + 3]);

    /* Syntactic error */
    if(!format || (source[0] != 'V' && source[0] == 'I')) return FAIL_PARSER_INVALID_FORMAT;

    /* Set values */
    source = tokens[index];
    points = resolveFloatNum(tokens[index + 3]);
    stop = resolveFloatNum(tokens[index + 2]);
    start = (scale == DEC_SCALE) ? resolveFloatNum(tokens[index + 1]):resolveIntNum(tokens[index + 1]);

    /* Timing checks */
    if((stop <= start) || (points <= 0)) return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
}

/*!
	@brief  Function verifies the syntax for a transient analysis spice card (.TRAN):
            => .TRAN  [tstep]  [tstop]
	Along with this, it returns the value contained in the <Value> token via the
	converted_val argument.

	@param      tokens    	The tokens that form the element.
	@param      step     	The time step point of the analysis.
	@param      tstop     	The end time of the analysis
	@param      tstart     	Starting time of the analysis, always set at 0.
	@return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parseTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart)
{
    if(tokens.size() != 3) return FAIL_PARSER_INVALID_FORMAT;

    /* Verify the validity of all tokens */
    bool format = IsValidFpValue(tokens[1]) && IsValidFpValue(tokens[2]);

    /* Verify */
    if(!format) return FAIL_PARSER_INVALID_FORMAT;

    /* Set values */
    tstart = 0;           // Default is to start at 0
    step = resolveFloatNum(tokens[1]);
    tstop = resolveFloatNum(tokens[2]);

    /* Verify */
	if((tstop <= tstart) || (step <= 0))
		return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
}

/*!
	@brief  Function verifies the syntaxes for a frequency analysis spice card (.AC):
            => .AC  DEC/LIN  [step]  [stop]  [start]
            => .AC  LOG  [points]  [stop]  [start]
	Along with this, it returns the appropriate values for each token.
	@param      tokens    	The tokens that form the element.
	@param      points     	Either the step (DEC/LIN scale) or the points per decade (LOG).
	@param      fstop     	The end voltage/current of the analysis.
	@param      fstart     	The start voltage/current of the analysis.
	@param		scale		The scale of the analysis (DEC or LOG).
	@return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parseACCard(std::vector<std::string> &tokens,
                                   double &points,
                                   double &fstop,
                                   double &fstart,
                                   as_scale_t &scale)
{
    if(tokens.size() != 5) return FAIL_PARSER_INVALID_FORMAT;

    /* Check type of scale */
    if(tokens[1] == "DEC" || tokens[1] == "LIN") scale = DEC_SCALE;
    else if (tokens[1] == "LOG") scale = LOG_SCALE;
    else return FAIL_PARSER_INVALID_FORMAT;

    /* Syntax verify */
    if(!(IsValidIntValue(tokens[2]) && IsValidFpValue(tokens[3]) && IsValidFpValue(tokens[4])))
        return FAIL_PARSER_INVALID_FORMAT;

    /* Set values */
    points = resolveFloatNum(tokens[2]);
    fstart = resolveFloatNum(tokens[3]);
    fstop = resolveFloatNum(tokens[4]);

	/* Timing checks */
	if((fstop <= fstart) || (points <= 0) || (fstart <= 0.0))
		return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

    return RETURN_SUCCESS;
}

/*!
	@brief  Function verifies the syntaxes for a plot/print spice card (.PLOT):
            => .PLOT  V(nodename1) I(Vsourcename1) ... V(nodenameN)
	Along with this, it returns the tokens (names of the nodes or sources).
	@param      tokens    		The tokens that form the element.
	@param      plot_nodes		The nodes to be plotted
	@param      plot_sources 	The sources to be plotted
	@return     RETURN_SUCCESS or appropriate failure code.
*/
return_codes_e parser::parsePLOTCard(std::vector<std::string> &tokens,
                                     std::vector<std::string> &plot_nodes,
                                     std::vector<std::string> &plot_sources)
{
	size_t tmp_size = tokens.size() - 1;
	size_t idx = 1;

	/* Since tokenization by the initial breakdown of the tokens, eliminates
	 * parentheses we do not have to resplit the tokens based on a different pattern/spec.
	 * The tokens are already in the form of {[V] nodename} or {[I] Vsourcename} */

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



/*!
    @brief      Function returns the unique ID of a node in the circuit
    (index in the MNA matrix), given the name of a node.
    @param      nodes     The nodes hashmap with {name, ID} associations .
    @param      name      The node name.
    @return     The node unique ID.
*/
IntTp parser::resolveNodeID(hashmap_str_t &nodes, const std::string &name)
{
    /* Ground node - Do not insert in map (-1 = tombstone) */
    if(name == "0") return -1;

    auto it = nodes.find(name);

    /* Node already has assigned ID */
    if(it != nodes.end()) return it->second;

    /* First time encountering this node - Insert in map */
    size_t sz = nodes.size();
    nodes[name] = sz;

    return sz;
}

/*!
    @brief      Function that creates the number conversion from
    string format to floating point.
    @param      num     The number in string format.
    @return     The number in floating point format.
*/
double parser::resolveFloatNum(const std::string &num)
{
    // TODO also check for spice modifiers
    return stod(num);
}

/*!
    @brief      Function that creates the number conversion from
    string format to integer.
    @param      num     The number in string format.
    @return     The number in floating point format.
*/
IntTp parser::resolveIntNum(const std::string &num)
{
    // TODO also check for spice modifiers
    return stod(num);
}



/*!
	@brief  Function verifies the basic element syntax of this type (Resistors, Coils, Capacitors):\n
            => [Element]  [V+]  [V-]  [Value]
	@param      tokens      The tokens that form the element.
	@return     Valid(true) syntax or not(false).
*/
bool parser::isValidTwoNodeElement(const std::vector<std::string> &tokens)
{
    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidFpValue(tokens[3]);

    return format;
}

/*!
	@brief  Function verifies a basic element syntax of this type (Voltage controlled sources):\n
            =>[Element]  [V+]  [V-]  [Vex]>  [Vex-]  [Value]
	@param      tokens    		  The tokens that form the element.
	@return     Valid(true) syntax or not(false).
*/
bool parser::isValidFourNodeElement(const std::vector<std::string> &tokens)
{
    if(tokens.size() != 6) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidNode(tokens[3]) &&
                  IsValidNode(tokens[4]) && IsValidFpValue(tokens[5]);

    return format;
}

/*!
    @brief  Function verifies the basic element syntax of this type (Current controlled sources):\n
            => [Element]  [V+]  [V-] [ElementSourceName]  [Value]
    @param      tokens      The tokens that form the element.
    @return     Valid(true) syntax or not(false).
*/
bool parser::isValidCurrentControlElement(const std::vector<std::string> &tokens)
{
    if(tokens.size() != 5) return false;

    /* Verify */
    bool format = IsValidName(tokens[0]) && IsValidNode(tokens[1]) &&
                  IsValidNode(tokens[2]) && IsValidName(tokens[3]) &&
                  IsValidFpValue(tokens[4]);

    return format;
}



/*!
	@brief      Internal function verifies that verifies the validity of a [NodeType]
	token and returns the result of the appraisal.
	@param      token    	The token.
	@return     Valid(true) syntax or not(false).
*/
bool parser::IsValidNode(const std::string &token)
{
    return std::regex_match(token, _alphanumeric_with_underscores);
}

/*!
	@brief      Internal function verifies that verifies the validity of an [ElementType]
	token and returns the result of the appraisal.
	@param      token       The token.
	@return     Valid(true) syntax or not(false).
*/
bool parser::IsValidName(const std::string &token)
{
    return std::regex_match(token, _alphanumeric_with_underscores);
}

/*!
    @brief      Internal function that verifies a floating point number
    and returns the result of the appraisal.
    @param      token       The token.
    @return     Valid(true) syntax or not(false).
*/
bool parser::IsValidFpValue(const std::string &token)
{
    return std::regex_match(token, _decimal_number);
}

/*!
    @brief      Internal function that verifies an integer number
    and returns the result of the appraisal.
    @param      token       The token.
    @return     Valid(true) syntax or not(false).
*/
bool parser::IsValidIntValue(const std::string &token)
{
    return std::regex_match(token, _integer_number);
}
