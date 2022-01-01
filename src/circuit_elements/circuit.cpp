#include <chrono>		/* For time reporting */
#include "circuit.hpp"

/*!
    @brief    Routine, that creates a circuit representation of the specified netlist
    given by the input argument (input_file). The line is parsed line by line and
    forms the necessary elements (R, L, C, etc..) and SPICE cards (.DC, .TRAN, etc..)
    for the circuit.
	@param	  input_file	The file that contains the SPICE netlist.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::CreateCircuit(std::fstream &input_file)
{
    using namespace std;
    using namespace chrono;

    string line;                                    /* Current line*/
    size_t linenum = 0;                             /* Linenumber */
    return_codes_e errcode = RETURN_SUCCESS;        /* Error code */
    syntax_parser syntax_match;                     /* Instantiate syntax matcher */
    vector<string> tokens;							/* Tokens produced for each line */

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
                errcode = syntax_match.Parse2NodeDevice(tokens, node2_base, this->_nodes, this->_element_names, id, true);
                this->_res.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'C':
            {
                id = this->_caps.size();
                errcode = syntax_match.Parse2NodeDevice(tokens, node2_base, this->_nodes, this->_element_names, id, true);
                this->_caps.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'L':
            {
                id = this->_coils.size();
                errcode = syntax_match.Parse2NodeDevice(tokens, node2_base, this->_nodes, this->_element_names, id, true);
                this->_coils.push_back({node2_base}); // C++17 aggregation initialize

                break;
            }
            case 'I':
            {
                id = this->_ics.size();
                errcode = syntax_match.Parse2NodeDevice(tokens, node2_base, this->_nodes, this->_element_names, id, false);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.ParseSourceSpec(tokens, source_spec_base);

                this->_ics.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case 'V':
            {
                id = this->_ivs.size();
                errcode = syntax_match.Parse2NodeDevice(tokens, node2_base, this->_nodes, this->_element_names, id, false);

                /* Continue parsing only in case of success */
                if(errcode == RETURN_SUCCESS) errcode = syntax_match.ParseSourceSpec(tokens, source_spec_base);

                this->_ivs.push_back({node2_base, source_spec_base}); // C++17 aggregation initialize
                break;
            }
            case '.':
            {
            	errcode = CreateSPICECard(tokens, syntax_match);
            	cout << "[INFO]: - At line " << linenum << ": Found SPICE CARD" << endl;

            	break;
            }
            case '*':
            {
                errcode = RETURN_SUCCESS;
                break;
            }
            default:
            {
                errcode = FAIL_PARSER_UKNOWN_ELEMENT;
                break;
            }
        }

        /* Early out, along with type, when dealing with an error */
        if(errcode != RETURN_SUCCESS)
        {
            cout << "[ERROR - " << errcode << "]: - At line " << linenum << ": " << line << endl;

//            cout << "Tokens with separator: " << endl;
//            for(string it: tokens) cout << "<" << it << ">" << endl;

            return errcode;
        }
    }

    /* Verify that circuit meets the criteria */
    errcode = verify();

    /* Measure the total time taken for parsing and verification */
    auto end_time = high_resolution_clock::now();

    /* Output information only in case of success */
    if(errcode == RETURN_SUCCESS)
    {
    	cout << endl << "************************************" << endl;
		cout << "************CIRCUIT INFO************" << endl;
		cout << "************************************" << endl;
		cout << "Load time: " << duration_cast<milliseconds>(end_time-begin_time).count() << "ms" << endl;
		cout << "Total lines: " << linenum << endl;
		cout << "************************************" << endl;
		cout << "Resistors: " << this->_res.size() << endl;
		cout << "Caps: " << this->_caps.size() << endl;
		cout << "Coils: " << this->_coils.size() << endl;
		cout << "ICS: " << this->_ics.size() << endl;
		cout << "IVS: " << this->_ivs.size() << endl;
		cout << "************************************" << endl;
		cout << "Simulation Type: " << this->_type << endl;
		cout << "Scale: " << this->_scale << endl;
		cout << "Total nodes to plot: " << this->_plot_nodes.size() << endl;
		cout << "Total sources to plot: " << this->_plot_sources.size() << endl;
		cout << "************************************" << endl << endl;
    }

    /* TODO - Add this to the report */
    int arr_ics[5] = {0};
    int arr_ivs[5] = {0};

	/* Calculate the IVS, ICS tran sources */
    for(auto &it : this->_ics) arr_ics[it.getType()]++;
    for(auto &it : this->_ivs) arr_ivs[it.getType()]++;

    cout << "I[C E S PW PU] " << arr_ics[0] <<  " " << arr_ics[1] <<  " " << arr_ics[2] <<  " " << arr_ics[3] <<  " " << arr_ics[4] << endl;
    cout << "V[C E S PW PU] " << arr_ivs[0] <<  " " << arr_ivs[1] <<  " " << arr_ivs[2] <<  " " << arr_ivs[3] <<  " " << arr_ivs[4] << endl;

    cout << "************************************" << endl << endl;

    return errcode;
}

/*!
    @brief    Internal routine, that parses and forms a SPICE card given the syntax matcher
    and the tokens of the element.
  	@param 	tokens	The tokens that contain the SPICE card.
  	@match	match	Syntax parser instantiation.
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::CreateSPICECard(std::vector<std::string> &tokens, syntax_parser &match)
{
	using namespace std;

	/* Copy the string from 1 position ahead of the '.' character */
	string spice_card = tokens[0].substr(1);

	/* Special case identified before everything else */
	if(spice_card == "PLOT" || spice_card == "PRINT")
	{
		return match.isValidSetPLOTCard(tokens, this->_plot_nodes, this->_plot_sources);
	}

	/* Base parameters for analysis */
	double start, stop, steps;
	return_codes_e errcode = RETURN_SUCCESS;

	/* Handle each analaysis */
	if(spice_card == "OP" && tokens.size() == 1)
	{
		this->_type = OP;
	}
	else if(spice_card == "DC")
	{
		this->_type = DC;
		errcode = match.isValidDCCard(tokens, steps, stop, start, this->_scale, this->_source);
	}
	else if(spice_card == "TRAN")
	{
		this->_type = TRAN;
		errcode = match.isValidTRANCard(tokens, steps, stop, start);
	}
	else if(spice_card == "AC")
	{
		this->_type = AC;
		errcode = match.isValidACCard(tokens, steps, stop, start, this->_scale);
	}
	else
	{
		return FAIL_PARSER_UKNOWN_SPICE_CARD;
	}

	/* Set the simulation parameters */
	this->_sim_step = steps;
	this->_sim_start = start;
	this->_sim_end = stop;

    return errcode;
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
    	{
    		errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
    		cout << "[ERROR - " << errcode << "]: <" << this->_source << "> does not exist in the circuit (DC CARD)" << endl;
    		return errcode;
    	}
    }

    /* Verify that each plot_source is correct and exists in the circuit */
    for(auto it = this->_plot_sources.begin(); it != this->_plot_sources.end(); it++)
    {
    	auto node_it = this->_element_names.find(*it);

		/* Does not exist in map */
		if(node_it == namemap_end)
		{
			errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
			cout << "[ERROR - " << errcode << "]: <" << *it << "> does not exist in the circuit (PLOT CARD)" << endl;
			return errcode;
		}
    }

    /* Verify that each plot_node is correct and exists in the circuit */
    for(auto it = this->_plot_nodes.begin(); it != this->_plot_nodes.end(); it++)
    {
    	auto node_it = this->_nodes.find(*it);

		/* Does not exist in map */
		if(node_it == nodemap_end)
		{
			errcode = FAIL_PARSER_ELEMENT_NOT_EXISTS;
			cout << "[ERROR - " << errcode << "]: <" << *it << "> does not exist in the circuit (PLOT CARD)" << endl;
			return errcode;
		}
    }

    /* Set valid flag */
    this->_valid = true;

    return RETURN_SUCCESS;
}
