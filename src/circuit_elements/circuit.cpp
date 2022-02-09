#include <chrono>		/* For time reporting */
#include <algorithm>
#include "circuit.hpp"

/*!
    @brief    Routine, that creates a circuit representation of the specified netlist
    given by the input argument (input_file). The line is parsed line by line and
    forms the necessary elements (R, L, C, etc..) and SPICE cards (.DC, .TRAN, etc..)
    for the circuit.
	@param	  input_file	The file that contains the SPICE netlist.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::create(std::string &input_file_name)
{
    using std::vector;
    using std::string;
    using std::ifstream;
    using std::cout;
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    /* Checks for file input */
    ifstream input_file(input_file_name);
    if(!input_file) return FAIL_LOADING_FILE;

    /* Clear the current circuit */
    this->clear();

    string line;                                    /* Current line*/
    size_t linenum = 0;                             /* Linenumber */
    return_codes_e errcode = RETURN_SUCCESS;        /* Error code */
    parser syntax_match;                            /* Instantiate parser engine */
    vector<string> tokens;							/* Tokens produced for each line */

    /* Info */
    cout << "\n[INFO]: Loading file...\n";

    /* Start of parsing - For statistics */
    auto begin_time = high_resolution_clock::now();

    while(getline(input_file, line))
    {
    	node2_device node2_base;
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
            case 'R':
            {
                id = this->_res.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                this->_res.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'C':
            {
                id = this->_caps.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                this->_caps.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'L':
            {
                id = this->_coils.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, true);
                this->_coils.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'I':
            {
                id = this->_ics.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, false);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.parseSourceSpec(tokens, source_spec_base);

                this->_ics.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case 'V':
            {
                id = this->_ivs.size();
                errcode = syntax_match.parse2NodeDevice(tokens, node2_base, this->_element_names, this->_nodes, id, false);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.parseSourceSpec(tokens, source_spec_base);

                this->_ivs.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case '.':
            {
            	errcode = createSPICECard(tokens, syntax_match);
            	cout << "[INFO]: - At line " << linenum << ": Found SPICE CARD\n";

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
            cout << "[ERROR - " << errcode << "]: At line " << linenum << ": " << line << "\n";
            input_file.close();
            return errcode;
        }
    }

    /* Verify that circuit meets the criteria */
    errcode = verify();

    /* Output information only in case of success */
    if(errcode == RETURN_SUCCESS)
    {
        /* Measure the total time taken */
        auto end_time = high_resolution_clock::now();

    	IntTp ics_count[TRANSIENT_SOURCE_TYPENUM] = {0};
    	IntTp ivs_count[TRANSIENT_SOURCE_TYPENUM] = {0};

    	cout << "************************************\n";
		cout << "************CIRCUIT INFO************\n";
		cout << "************************************\n";
		cout << "Load time: " << duration_cast<milliseconds>(end_time-begin_time).count() << "ms\n";
		cout << "Total lines: " << linenum << "\n";
		cout << "************************************\n";
		cout << "Resistors: " << this->_res.size() << "\n";
		cout << "Caps: " << this->_caps.size() << "\n";
		cout << "Coils: " << this->_coils.size() << "\n";

		cout << "ICS: " << this->_ics.size() << "\n";
		for(auto &it : this->_ics) ics_count[it.getType()]++;
		cout << "\tConstant: " << ics_count[CONSTANT_SOURCE] << "\n";
		cout << "\tExp: " << ics_count[EXP_SOURCE] << "\n";
		cout << "\tSine: " << ics_count[SINE_SOURCE] << "\n";
		cout << "\tPWL: " << ics_count[PWL_SOURCE] << "\n";
		cout << "\tPulse: " << ics_count[PULSE_SOURCE] << "\n";

		cout << "IVS: " << this->_ivs.size() << "\n";
		for(auto &it : this->_ivs) ivs_count[it.getType()]++;
		cout << "\tConstant: " << ivs_count[CONSTANT_SOURCE] << "\n";
		cout << "\tExp: " << ivs_count[EXP_SOURCE] << "\n";
		cout << "\tSine: " << ivs_count[SINE_SOURCE] << "\n";
		cout << "\tPWL: " << ivs_count[PWL_SOURCE] << "\n";
		cout << "\tPulse: " << ivs_count[PULSE_SOURCE] << "\n";

		cout << "************************************\n";
		cout << "Simulation Type: " << this->_type << "\n";
		cout << "Scale: " << this->_scale << "\n";
		cout << "Memory mode: " << this->_mem_save_mode << "\n";
		cout << "ODE method: " << this->_ode_method << "\n";
		cout << "Total nodes to plot: " << this->_plot_nodes.size() << "\n";
		cout << "Total sources to plot: " << this->_plot_sources.size() << "\n";
		cout << "************************************\n\n";
    }

    input_file.close();
    return errcode;
}

/*!
    @brief    Internal routine, that parses and forms a SPICE card given the syntax matcher
    and the tokens of the element.
  	@param 	tokens	The tokens that contain the SPICE card.
  	@param	match	Syntax parser instantiation.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::createSPICECard(std::vector<std::string> &tokens, parser &match)
{
	using namespace std;

	/* Check whether we have '.' character */
	string spice_card;
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
return_codes_e Circuit::setCircuitOptions(std::vector<std::string> &tokens)
{
    auto it = tokens.begin() + 1;
    bool mem_save_found = false;
    bool integr_found = false;

    /* Iteratively find every option card */
    while(it != tokens.end())
    {
        if(*it == "MEM_SAVE" && !mem_save_found)
        {
            this->_mem_save_mode = true;
            mem_save_found = true;
        }
        else if(*it == "GEAR2" && !integr_found)
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
  	  - Plot nodes for a plot card already exists in the circuit
  	  - DC analysis source already exists in the circuit (if any DC analysis is active)
  	  - Set the circuit dimension and the source offset in the MNA equivalent matrix.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::verify(void)
{
	using namespace std;

	return_codes_e errcode;
    auto namemap_end = this->_element_names.end();
    auto nodemap_end = this->_nodes.end();

    /* After parsing the file, in case of DC analysis verify that the simulated source exists */
    if(this->_type == DC)
    {
    	auto name_it = this->_element_names.find(this->_source);

    	/* Does not exist in map */
    	if(name_it == namemap_end)
    	{// TODO - Transfer to error function outside
    		errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
    		cout << "[ERROR - " << errcode << "]: Element <" << this->_source << "> (DC CARD)" << endl;
    		return errcode;
    	}
    }

    /* Verify that each plot_source is correct and exists in the circuit */
    for(auto it = this->_plot_sources.begin(); it != this->_plot_sources.end(); it++)
    {
    	auto node_it = this->_element_names.find(*it);

		/* Does not exist in map */
		if(node_it == namemap_end)
		{// TODO - Transfer to error function outside
			errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
			cout << "[ERROR - " << errcode << "]: Element <" << *it << "> (PLOT CARD)" << endl;
			return errcode;
		}
    }

    /* Verify that each plot_node is correct and exists in the circuit */
    for(auto it = this->_plot_nodes.begin(); it != this->_plot_nodes.end(); it++)
    {
    	auto node_it = this->_nodes.find(*it);

		/* Does not exist in map */
		if(node_it == nodemap_end)
		{// TODO - Transfer to error function outside
			errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
			cout << "[ERROR - " << errcode << "]: Element <" << *it << "> (PLOT CARD)" << endl;
			return errcode;
		}
    }

    /* Set valid flag */
    this->_valid = true;

    return RETURN_SUCCESS;
}



/*!
    @brief    Internal debug routine, that recreates the nodesmap, based on
    the ordering defined by debug_circuit(). Used only for debugging.
    @param  element   The current element whose nodes we insert in the map
*/
void Circuit::debug_insert_nodes(node2_device &element)
{
    auto &pos_name = element.getPosNode();
    auto &neg_name = element.getNegNode();
    auto new_posID = element.getPosNodeID();
    auto new_negID = element.getNegNodeID();

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
    @brief    Internal routine that reorders the elements in their containers,
    specified by the comparator and the recreate the elements and nodes maps.
    Used only for debugging.
*/
void Circuit::debug_circuit(void)
{
    /* Perform ordering of the elements in their containers by name and
     * then order the nodes unique numbering by inserting in this exact order:
     * IVS - Coils - ICS - Res - Caps */
	using namespace std;

	vector<Coil> tmp_coils = this->_coils;
	vector<Capacitor> tmp_caps = this->_caps;
	vector<Resistor> tmp_res = this->_res;
	vector<ivs> tmp_ivs = this->_ivs;
	vector<ics> tmp_ics = this->_ics;

	/* Sort by name */
	sort(tmp_coils.begin(), tmp_coils.end());
	sort(tmp_caps.begin(), tmp_caps.end());
	sort(tmp_res.begin(), tmp_res.end());
	sort(tmp_ivs.begin(), tmp_ivs.end());
	sort(tmp_ics.begin(), tmp_ics.end());

	/* Reset both maps */
	this->_element_names.clear();
	auto &elementmap = this->_element_names;
	for(size_t i = 0; i < tmp_ivs.size(); i++) elementmap[tmp_ivs[i].getName()] = i;
	for(size_t i = 0; i < tmp_coils.size(); i++) elementmap[tmp_coils[i].getName()] = i;
	for(size_t i = 0; i < tmp_ics.size(); i++) elementmap[tmp_ics[i].getName()] = i;
	for(size_t i = 0; i < tmp_res.size(); i++) elementmap[tmp_res[i].getName()] = i;
	for(size_t i = 0; i < tmp_caps.size(); i++) elementmap[tmp_caps[i].getName()] = i;

    /* Recreate the nodesmap */
	this->_nodes.clear();
    for(auto &it : tmp_ivs) debug_insert_nodes(it);
    for(auto &it : tmp_coils) debug_insert_nodes(it);
    for(auto &it : tmp_ics) debug_insert_nodes(it);
    for(auto &it : tmp_res) debug_insert_nodes(it);
    for(auto &it : tmp_caps) debug_insert_nodes(it);

	/* Output */
    this->_coils = tmp_coils;
    this->_caps = tmp_caps;
    this->_res = tmp_res;
    this->_ivs = tmp_ivs;
    this->_ics = tmp_ics;

	cout.precision(12);
	cout << std::fixed;
	for(auto &it : tmp_ivs) cout << it;
	for(auto &it : tmp_coils) cout << it;
	for(auto &it : tmp_ics) cout << it;
	for(auto &it : tmp_res) cout << it;
	for(auto &it : tmp_caps) cout << it;
}
