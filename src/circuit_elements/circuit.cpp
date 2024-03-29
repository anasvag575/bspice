#include <chrono>		/* For time reporting */
#include <algorithm>
#include "circuit.hpp"
#include <unordered_map>    /* TODO - For multimap */



/*!
    @brief    Get the resistors in the circuit.
    @return   The resistors vector.
*/
const std::vector<resistor> &circuit::Resistors(void) noexcept { return _res; }

/*!
    @brief    Get the capacitors in the circuit.
    @return   The capacitors vector.
*/
const std::vector<capacitor> &circuit::Capacitors(void) noexcept { return _caps; }

/*!
    @brief    Get the coils in the circuit.
    @return   The coils vector.
*/
const std::vector<coil> &circuit::Coils(void) noexcept { return _coils; }

/*!
    @brief    Get the ICS in the circuit.
    @return   The ICS vector.
*/
const std::vector<ics> &circuit::ICS(void) noexcept { return _ics; }

/*!
    @brief    Get the IVS in the circuit.
    @return   The IVS vector.
*/
const std::vector<ivs> &circuit::IVS(void) noexcept { return _ivs; }

/*!
    @brief    Get the VCVS in the circuit.
    @return   The VCVS vector.
*/
const std::vector<vcvs> &circuit::VCVS(void) noexcept { return _vcvs; }

/*!
    @brief    Get the VCCS in the circuit.
    @return   The VCCS vector.
*/
const std::vector<vccs> &circuit::VCCS(void) noexcept { return _vccs; }

/*!
    @brief    Get the CCVS in the circuit.
    @return   The CCVS vector.
*/
const std::vector<ccvs> &circuit::CCVS(void) noexcept { return _ccvs; }

/*!
    @brief    Get the CCCS in the circuit.
    @return   The CCCS vector.
*/
const std::vector<cccs> &circuit::CCCS(void) noexcept { return _cccs; }

/*!
    @brief    Get the nodes map (<NodeName, NodeNum> pairs).
    @return   The map.
*/
const hashmap_str_t &circuit::Nodes(void) noexcept { return _nodes; }

/*!
    @brief    Get the elements map (<ElmementName, ElementID> pairs).
    @return   The map.
*/
const hashmap_str_t &circuit::ElementNames(void) noexcept { return _element_names; }

/*!
    @brief    Get the nodes to be plotted in the circuit.
    @return   The vector.
*/
const std::vector<std::string> &circuit::PlotNodes(void) noexcept { return _plot_nodes; }

/*!
    @brief    Get the sources to be plotted in the circuit.
    @return   The vector.
*/
const std::vector<std::string> &circuit::PlotSources(void) noexcept { return _plot_sources; }

/*!
    @brief    Get the DC source for analysis.
    @return   The DC source name.
*/
const std::string &circuit::DCSource(void) noexcept { return _source; }

/*!
    @brief    Get the simulation start value.
    @return   The start value.
*/
double circuit::SimStart(void) noexcept { return _sim_start; }

/*!
    @brief    Get the simulation end value.
    @return   The end value.
*/
double circuit::SimEnd(void) noexcept { return _sim_end; }

/*!
    @brief    Get the simulation step.
    @return   The step value.
*/
double circuit::SimStep(void) noexcept { return _sim_step; }

/*!
    @brief    Get the analysis type.
    @return   The type.
*/
analysis_t circuit::AnalysisType(void) noexcept { return _type; }

/*!
    @brief    Get the analysis scale.
    @return   The scale.
*/
as_scale_t circuit::AnalysisScale(void) noexcept { return _scale; }

/*!
    @brief    Get the ODE method to be used for transient.
    @return   The method.
*/
ODE_meth_t circuit::ODEMethod(void) noexcept { return _ode_method; }

/*!
    @brief    Returns the last error during parsing of the netlist.
    @return   Error code.
*/
return_codes_e circuit::errcode(void) noexcept { return _errcode; }

/*!
    @brief    Check if the current circuit is valid.
    @return   Boolean.
*/
bool circuit::valid(void) noexcept { return _errcode == RETURN_SUCCESS;}

/*!
    @brief    Clear circuit redundant information, optional use before entering simulation.
*/
void circuit::clear(void)
{
    /* Release vector memory */
    std::vector<resistor>().swap(_res);
    std::vector<capacitor>().swap(_caps);
    std::vector<coil>().swap(_coils);
    std::vector<ics>().swap(_ics);
    std::vector<ivs>().swap(_ivs);
    std::vector<vcvs>().swap(_vcvs);
    std::vector<vccs>().swap(_vccs);
    std::vector<ccvs>().swap(_ccvs);
    std::vector<cccs>().swap(_cccs);

    /* Release maps memory */
    hashmap_str_t().swap(_element_names);
    hashmap_str_t().swap(_nodes);

    /* Leave out the plot names since they are needed... */
}



/*!
    @brief    Routine, that creates a circuit representation of the specified netlist
    given by the input argument (input_file). The file is parsed line by line and
    forms the necessary SPICE elements and cards of the circuit.
    @param    input_file_name    The file that contains the SPICE netlist.
*/
circuit::circuit(const std::string &input_file_name)
{
    /* Checks for file input */
    std::ifstream input_file(input_file_name);
    if(!input_file)
    {
        _errcode = FAIL_LOADING_FILE;
        return;
    }

    std::string line;                               /* Current line*/
    size_t linenum = 0;                             /* Linenumber */
    return_codes_e errcode = RETURN_SUCCESS;        /* Error code */
    parser syntax_match;                            /* Instantiate parser engine */
    std::vector<std::string> tokens;                /* Tokens produced for each line */

    /* Info */
    std::cout << "\n[INFO]: Loading file...\n";

    /* Default initialize values in case netlist does not do so */
    this->_ode_method = BACKWARDS_EULER;
    this->_scale = DEC_SCALE;
    this->_type = OP;
    this->_errcode = FAIL_LOADING_FILE;
    this->_sim_step = 0;
    this->_sim_start = 0;
    this->_sim_end = 0;

    /* Start of parsing - For statistics */
    auto begin_time = std::chrono::high_resolution_clock::now();

    while(getline(input_file, line))
    {
        node2s_device node2s_base;
        node2_device node2_base;
        node4_device node4_base;
        source_spec source_spec_base;
        size_t id;

        /* Keep the line number for debugging */
        linenum++;

        /* Tokenize the line, if empty go to next */
        if(!syntax_match.tokenizer(line, tokens)) continue;

        /* Check the first character of the first string - Based on this we move to the next action */
        const char c = (*tokens.begin())[0];

        switch(c)
        {
            case 'R': // Resistors
            {
                id = this->_res.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                node2_base.setLinenum(linenum);
                this->_res.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'C': // Capacitors
            {
                id = this->_caps.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                node2_base.setLinenum(linenum);
                this->_caps.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'L': // Coils
            {
                id = this->_coils.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                node2_base.setLinenum(linenum);
                this->_coils.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'I': // Independent current sources
            {
                id = this->_ics.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, false);
                node2_base.setLinenum(linenum);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.parseSourceSpec(tokens, source_spec_base);

                this->_ics.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case 'V': // Independent voltage sources
            {
                id = this->_ivs.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, false);
                node2_base.setLinenum(linenum);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.parseSourceSpec(tokens, source_spec_base);

                this->_ivs.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case 'E': // Voltage controlled voltage sources
            {
                id = this->_vcvs.size();
                errcode = syntax_match.parse4NodeDevice(tokens, node4_base, this->_element_names, this->_nodes, id);
                node4_base.setLinenum(linenum);

                this->_vcvs.push_back({node4_base}); // C++17 aggregation initialize
                break;
            }
            case 'G': // Voltage controlled current sources
            {
                id = this->_vccs.size();
                errcode = syntax_match.parse4NodeDevice(tokens, node4_base, this->_element_names, this->_nodes, id);
                node4_base.setLinenum(linenum);

                this->_vccs.push_back({node4_base}); // C++17 aggregation initialize
                break;
            }
            case 'H': // Current controlled voltage sources
            {
                id = this->_ccvs.size();
                errcode = syntax_match.parse2SNodeDevice(tokens, node2s_base, this->_element_names, this->_nodes, id);
                node2s_base.setLinenum(linenum);

                this->_ccvs.push_back({node2s_base}); // C++17 aggregation initialize
                break;
            }
            case 'F': // Current controlled current sources
            {
                id = this->_cccs.size();
                errcode = syntax_match.parse2SNodeDevice(tokens, node2s_base, this->_element_names, this->_nodes, id);
                node2s_base.setLinenum(linenum);

                this->_cccs.push_back({node2s_base}); // C++17 aggregation initialize
                break;
            }
            case '.':
            {
                errcode = SPICECard(tokens, syntax_match);
                std::cout << "[INFO]: - At line " << linenum << ": Found SPICE CARD\n";

                break;
            }
            case '*':
            {
                errcode = RETURN_SUCCESS;
                break;
            }
            default:
            {
                errcode = FAIL_PARSER_UNKNOWN_ELEMENT;
                break;
            }
        }

        /* Early out, along with type, when dealing with an error */
        if(errcode != RETURN_SUCCESS)
        {
            std::cout << "[ERROR - " << errcode << "]: At line " << linenum << ": " << line << "\n";
            input_file.close();
            this->_errcode = errcode;
            return;
        }
    }

    /* Verify that circuit meets the criteria */
    input_file.close();
    errcode = verify();
    this->_errcode = errcode;
    if(errcode != RETURN_SUCCESS) return;

    /* Verify the circuit topology */
    errcode = topology();
    this->_errcode = errcode;

    /* Output information only in case of success */
    if(errcode == RETURN_SUCCESS)
    {
        /* Measure the total time taken */
        auto end_time = std::chrono::high_resolution_clock::now();

        IntTp ics_count[TRANSIENT_SOURCE_TYPENUM] = {0};
        IntTp ivs_count[TRANSIENT_SOURCE_TYPENUM] = {0};

        std::cout << "************************************\n";
        std::cout << "************CIRCUIT INFO************\n";
        std::cout << "************************************\n";
        std::cout << "Load time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end_time-begin_time).count() << "ms\n";
        std::cout << "Total lines: " << linenum << "\n";
        std::cout << "************************************\n";
        std::cout << "Resistors: " << this->_res.size() << "\n";
        std::cout << "Caps: " << this->_caps.size() << "\n";
        std::cout << "Coils: " << this->_coils.size() << "\n";
        std::cout << "VCVS: " << this->_vcvs.size() << "\n";
        std::cout << "VCCS: " << this->_vccs.size() << "\n";
        std::cout << "CCVS: " << this->_ccvs.size() << "\n";
        std::cout << "CCCS: " << this->_cccs.size() << "\n";

        std::cout << "ICS: " << this->_ics.size() << "\n";
        for(auto &it : this->_ics) ics_count[it.Type()]++;
        std::cout << "\tConstant: " << ics_count[CONSTANT_SOURCE] << "\n";
        std::cout << "\tExp: " << ics_count[EXP_SOURCE] << "\n";
        std::cout << "\tSine: " << ics_count[SINE_SOURCE] << "\n";
        std::cout << "\tPWL: " << ics_count[PWL_SOURCE] << "\n";
        std::cout << "\tPulse: " << ics_count[PULSE_SOURCE] << "\n";

        std::cout << "IVS: " << this->_ivs.size() << "\n";
        for(auto &it : this->_ivs) ivs_count[it.Type()]++;
        std::cout << "\tConstant: " << ivs_count[CONSTANT_SOURCE] << "\n";
        std::cout << "\tExp: " << ivs_count[EXP_SOURCE] << "\n";
        std::cout << "\tSine: " << ivs_count[SINE_SOURCE] << "\n";
        std::cout << "\tPWL: " << ivs_count[PWL_SOURCE] << "\n";
        std::cout << "\tPulse: " << ivs_count[PULSE_SOURCE] << "\n";

        std::cout << "************************************\n";
        std::cout << "Simulation Type: " << this->_type << "\n";
        std::cout << "Scale: " << this->_scale << "\n";
        std::cout << "ODE method: " << this->_ode_method << "\n";
        std::cout << "Total nodes to plot: " << this->_plot_nodes.size() << "\n";
        std::cout << "Total sources to plot: " << this->_plot_sources.size() << "\n";
        std::cout << "************************************\n\n";
    }
}

/*!
    @brief    Internal routine, that parses and forms a SPICE card given the syntax matcher
    and the tokens of the element.
  	@param 	tokens	The tokens that contain the SPICE card.
  	@param	match	Syntax parser instantiation.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e circuit::SPICECard(std::vector<std::string> &tokens, parser &match)
{
	/* Check whether we have '.' character */
	std::string spice_card;
	(tokens[0][0] == '.') ? spice_card = tokens[0].substr(1) : spice_card = tokens[0];

	/* Special case identified before everything else */
	if(spice_card == "PLOT" || spice_card == "PRINT")
	{
		return match.parsePLOTCard(tokens, this->_plot_nodes, this->_plot_sources);
	}
	else if(spice_card == "OPTIONS") /* Means we parse simulator/circuit options and set them directly */
	{
	    return setCircuitOptions(tokens);
	}

	/* Base parameters for analysis */
	double start = 0, stop = 0, steps = 0;
	return_codes_e errcode = RETURN_SUCCESS;

	/* Handle each analaysis */
	if(spice_card == "OP" && tokens.size() == 1)
	{
		this->_type = OP;
	}
	else if(spice_card == "DC")
	{
		this->_type = DC;
		errcode = match.parseDCCard(tokens, steps, stop, start, this->_scale, this->_source);
	}
	else if(spice_card == "TRAN")
	{
		this->_type = TRAN;
		errcode = match.parseTRANCard(tokens, steps, stop, start);
	}
	else if(spice_card == "AC")
	{
		this->_type = AC;
		errcode = match.parseACCard(tokens, steps, stop, start, this->_scale);
	}
	else
	{
		return FAIL_PARSER_UNKNOWN_SPICE_CARD;
	}

	/* Set the simulation parameters */
	this->_sim_step = steps;
	this->_sim_start = start;
	this->_sim_end = stop;

    return errcode;
}

/*!
    @brief    Internal routine, that parses the options given by an OPTIONS spice card.
    Since there is no need for tokanization or special pattern matching, this is here
    inside the circuit class.
    @param  tokens  The tokens that contain the OPTIONS card.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e circuit::setCircuitOptions(std::vector<std::string> &tokens)
{
    auto it = tokens.begin() + 1;
    bool integr_found = false;

    /* Iteratively find every option card */
    while(it != tokens.end())
    {
        if(*it == "GEAR2" && !integr_found)
        {
            this->_ode_method = GEAR2;
            integr_found = true;
        }
        else if(*it == "EULER" && !integr_found)
        {
            this->_ode_method = BACKWARDS_EULER;
            integr_found = true;
        }
        else if(*it == "TRAP" && !integr_found)
        {
            this->_ode_method = TRAPEZOIDAL;
            integr_found = true;
        }
        else
        {
            return FAIL_PARSER_UKNOWN_OPTION_OR_REPETITION;
        }

        it++;
    }

    return RETURN_SUCCESS;
}

/*!
    @brief    Internal routine, that verifies the following for a given circuit:
  	  - Plot nodes for a plot card already exists in the circuit.
  	  - DC analysis source already exists in the circuit (if any DC analysis is active)
  	  - Set the circuit dimension and the source offset in the MNA equivalent matrix.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e circuit::verify(void)
{
    auto namemap_end = this->_element_names.end();
    auto nodemap_end = this->_nodes.end();

    /* After parsing the file, in case of DC analysis verify that the simulated source exists */
    if(this->_type == DC)
    {
    	auto name_it = this->_element_names.find(this->_source);

    	/* Does not exist in map */
    	if(name_it == namemap_end)
    	{// TODO - Transfer to error function outside
    		std::cout << "[ERROR - " << FAIL_PARSER_ELEMENT_NOT_EXISTS << "]: Element <" << this->_source << "> (DC CARD)" << std::endl;
    		return FAIL_PARSER_ELEMENT_NOT_EXISTS;
    	}
    }

    /* Verify that each plot_source is correct and exists in the circuit */
    for(auto it = this->_plot_sources.begin(); it != this->_plot_sources.end(); it++)
    {
    	auto node_it = this->_element_names.find(*it);

		/* Does not exist in map */
		if(node_it == namemap_end)
		{// TODO - Transfer to error function outside
			std::cout << "[ERROR - " << FAIL_PARSER_ELEMENT_NOT_EXISTS << "]: Element <" << *it << "> (PLOT CARD)" << std::endl;
			return FAIL_PARSER_ELEMENT_NOT_EXISTS;
		}
    }

    /* Verify that each plot_node is correct and exists in the circuit */
    for(auto it = this->_plot_nodes.begin(); it != this->_plot_nodes.end(); it++)
    {
    	auto node_it = this->_nodes.find(*it);

		/* Does not exist in map */
		if(node_it == nodemap_end)
		{// TODO - Transfer to error function outside
			std::cout << "[ERROR - " << FAIL_PARSER_ELEMENT_NOT_EXISTS << "]: Element <" << *it << "> (PLOT CARD)" << std::endl;
			return FAIL_PARSER_ELEMENT_NOT_EXISTS;
		}
    }

    /* Verify that each CCVS depended source exists */
    for(auto &it : this->_ccvs)
    {
        auto map_it = this->_element_names.find(it.SourceName());

        /* Does not exist in map or wrong type of element */
        if(map_it == namemap_end)
        {// TODO - Transfer to error function outside
            std::cout << "[ERROR - " << FAIL_PARSER_ELEMENT_NOT_EXISTS << "]: Element <" << it.SourceName() << "> (CCVS DEPENDENCY)" << std::endl;
            return FAIL_PARSER_ELEMENT_NOT_EXISTS;
        }

        /* Set the source ID */
        it.SetSourceID(map_it->second);
    }

    /* Verify that each CCCS depended source exists */
    for(auto &it : this->_cccs)
    {
        auto map_it = this->_element_names.find(it.SourceName());

        /* Does not exist in map or wrong type of element */
        if(map_it == namemap_end)
        {// TODO - Transfer to error function outside
            std::cout << "[ERROR - " << FAIL_PARSER_ELEMENT_NOT_EXISTS << "]: Element <" << it.SourceName() << "> (CCCS DEPENDENCY)" << std::endl;
            return FAIL_PARSER_ELEMENT_NOT_EXISTS;
        }

        /* Set the source ID */
        it.SetSourceID(map_it->second);
    }

    return RETURN_SUCCESS;
}

/*!
    @brief    Internal routine, that performs topology checks on the circuit:
    // TODO - Fill up
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e circuit::topology(void)
{
    /* Rules of circuit (in order to not generate warnings!):
     * 1) Short circuits are eliminated from parser and checked there!
     * 2)   */

    /* Create a multimap of nodes to element names */
    std::unordered_multimap<IntTp, IntTp> nodes_to_dev;
    size_t count = 0;

    for(auto &it : _ics){ }
    for(auto &it : _cccs){ }
    for(auto &it : _vccs){ }



    /* Current sources in the same branch */

    /* IVS - IVS on the exact same nodes (check with ground) */

    /* Path to ground */


    return RETURN_SUCCESS;
}



/*!
    @brief    Internal debug routine, that recreates the nodesmap, based on
    the ordering defined by debug_circuit(). Used only for debugging.
    @param  element   The current element whose nodes we insert in the map
*/
void circuit::debug_insert_nodes(node2_device &element)
{
    auto &pos_name = element.PosNode();
    auto &neg_name = element.NegNode();
    auto new_posID = element.PosNodeID();
    auto new_negID = element.NegNodeID();

    /* Ground node - Do not insert in map (-1) */
    if(pos_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(pos_name);

        if(it != _nodes.end())
        {
            new_posID = it->second;
        }
        else /* First time encountering this node */
        {
            new_posID = _nodes.size();
            _nodes[pos_name] = new_posID; /* Also insert in map */
        }
    }

    /* Ground node - Do not insert in map (-1) */
    if(neg_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(neg_name);

        if(it != _nodes.end())
        {
            new_negID = it->second;
        }
        else
        {
            new_negID = _nodes.size();
            _nodes[neg_name] = new_negID;
        }
    }

    /* Update */
    element.setNodeIDs(new_posID, new_negID);
}

/*!
    @brief    Internal debug routine, that recreates the nodesmap, based on
    the ordering defined by debug_circuit(). Used only for debugging.
    @param  element   The current element whose nodes we insert in the map
*/
void circuit::debug_insert_nodes(node4_device &element)
{
    auto &pos_name = element.PosNode();
    auto &neg_name = element.NegNode();
    auto &DepPos_name = element.DepPosNode();
    auto &DepNeg_name = element.DepNegNode();
    auto new_posID = element.PosNodeID();
    auto new_negID = element.NegNodeID();
    auto new_DepPosID = element.DepPosNodeID();
    auto new_DepNegID = element.DepNegNodeID();

    /* Ground node - Do not insert in map (-1) */
    if(pos_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(pos_name);

        if(it != _nodes.end())
        {
            new_posID = it->second;
        }
        else /* First time encountering this node */
        {
            new_posID = _nodes.size();
            _nodes[pos_name] = new_posID; /* Also insert in map */
        }
    }

    /* Ground node - Do not insert in map (-1) */
    if(neg_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(neg_name);

        if(it != _nodes.end())
        {
            new_negID = it->second;
        }
        else
        {
            new_negID = _nodes.size();
            _nodes[neg_name] = new_negID;
        }
    }

    /* Ground node - Do not insert in map (-1) */
    if(DepPos_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(DepPos_name);

        if(it != _nodes.end())
        {
            new_DepPosID = it->second;
        }
        else /* First time encountering this node */
        {
            new_DepPosID = _nodes.size();
            _nodes[DepPos_name] = new_DepPosID; /* Also insert in map */
        }
    }

    /* Ground node - Do not insert in map (-1) */
    if(DepNeg_name != "0")
    {
        /* Normal node */
        auto it = _nodes.find(DepNeg_name);

        if(it != _nodes.end())
        {
            new_DepNegID = it->second;
        }
        else
        {
            new_DepNegID = _nodes.size();
            _nodes[DepNeg_name] = new_DepNegID;
        }
    }

    /* Update */
    element.setNodeIDs(new_posID, new_negID);
    element.setDepNodeIDs(new_DepPosID, new_DepNegID);
}

/*!
    @brief    Internal routine that reorders the elements in their containers,
    specified by the comparator and the recreate the elements and nodes maps.
    Used only for debugging.
*/
void circuit::debug_circuit(void)
{
    /* Perform ordering of the elements in their containers by name and
     * then order the nodes unique numbering by inserting in this exact order:
     * IVS - Coils - ICS - Res - Caps - VCCS - VCVS */
    std::vector<coil> tmp_coils = this->_coils;
    std::vector<capacitor> tmp_caps = this->_caps;
    std::vector<resistor> tmp_res = this->_res;
    std::vector<ivs> tmp_ivs = this->_ivs;
    std::vector<ics> tmp_ics = this->_ics;
    std::vector<vccs> tmp_vccs = this->_vccs;
    std::vector<vcvs> tmp_vcvs = this->_vcvs;
    std::vector<cccs> tmp_cccs = this->_cccs;
    std::vector<ccvs> tmp_ccvs = this->_ccvs;

	/* Sort by name */
	sort(tmp_coils.begin(), tmp_coils.end());
	sort(tmp_caps.begin(), tmp_caps.end());
	sort(tmp_res.begin(), tmp_res.end());
	sort(tmp_ivs.begin(), tmp_ivs.end());
	sort(tmp_ics.begin(), tmp_ics.end());
    sort(tmp_vccs.begin(), tmp_vccs.end());
    sort(tmp_vcvs.begin(), tmp_vcvs.end());
    sort(tmp_cccs.begin(), tmp_cccs.end());
    sort(tmp_ccvs.begin(), tmp_ccvs.end());

	/* Reset both maps */
	this->_element_names.clear();
	auto &elementmap = this->_element_names;
	for(size_t i = 0; i < tmp_ivs.size(); i++) elementmap[tmp_ivs[i].Name()] = i;
	for(size_t i = 0; i < tmp_coils.size(); i++) elementmap[tmp_coils[i].Name()] = i;
	for(size_t i = 0; i < tmp_ics.size(); i++) elementmap[tmp_ics[i].Name()] = i;
	for(size_t i = 0; i < tmp_res.size(); i++) elementmap[tmp_res[i].Name()] = i;
	for(size_t i = 0; i < tmp_caps.size(); i++) elementmap[tmp_caps[i].Name()] = i;
    for(size_t i = 0; i < tmp_vccs.size(); i++) elementmap[tmp_vccs[i].Name()] = i;
    for(size_t i = 0; i < tmp_vcvs.size(); i++) elementmap[tmp_vcvs[i].Name()] = i;
    for(size_t i = 0; i < tmp_cccs.size(); i++) elementmap[tmp_cccs[i].Name()] = i;
    for(size_t i = 0; i < tmp_ccvs.size(); i++) elementmap[tmp_ccvs[i].Name()] = i;

    /* Recreate the nodesmap */
	this->_nodes.clear();
    for(auto &it : tmp_ivs) debug_insert_nodes(it);
    for(auto &it : tmp_coils) debug_insert_nodes(it);
    for(auto &it : tmp_ics) debug_insert_nodes(it);
    for(auto &it : tmp_res) debug_insert_nodes(it);
    for(auto &it : tmp_caps) debug_insert_nodes(it);
    for(auto &it : tmp_vccs) debug_insert_nodes(it);
    for(auto &it : tmp_vcvs) debug_insert_nodes(it);
    for(auto &it : tmp_cccs) debug_insert_nodes(it);
    for(auto &it : tmp_ccvs) debug_insert_nodes(it);

	/* Output */
    this->_coils = tmp_coils;
    this->_caps = tmp_caps;
    this->_res = tmp_res;
    this->_ivs = tmp_ivs;
    this->_ics = tmp_ics;
    this->_vccs = tmp_vccs;
    this->_vcvs = tmp_vcvs;
    this->_cccs = tmp_cccs;
    this->_ccvs = tmp_ccvs;

	std::cout.precision(12);
	std::cout << std::fixed;
	for(auto &it : tmp_ivs) std::cout << it;
	for(auto &it : tmp_coils) std::cout << it;
	for(auto &it : tmp_ics) std::cout << it;
	for(auto &it : tmp_res) std::cout << it;
	for(auto &it : tmp_caps) std::cout << it;
    for(auto &it : tmp_vccs) std::cout << it;
    for(auto &it : tmp_vcvs) std::cout << it;
    for(auto &it : tmp_cccs) std::cout << it;
    for(auto &it : tmp_ccvs) std::cout << it;
}
