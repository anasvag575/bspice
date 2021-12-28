#include <chrono>		/* For time reporting */
#include "circuit.hpp"
#include "math_util.hpp"

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

    string line;                                    /* Current line*/
    size_t linenum = 0;                             /* Linenumber */
    return_codes_e errcode = RETURN_SUCCESS;        /* Error code */
    syntax_parser syntax_match;                     /* Instantiate syntax matcher */
    vector<string> tokens;							/* Tokens produced for each line */

    /* Start of parsing - For statistics */
    auto begin = chrono::high_resolution_clock::now();

    while(getline(input_file, line))
    {
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
                Resistor tmp_res;
                errcode = tmp_res.ParseResistor(tokens, syntax_match, this->_nodes, this->_base_element_names, this->_res.size());
                this->_res.push_back(tmp_res);

                break;
            }
            case 'C':
            {
                Capacitor tmp_cap;
                errcode = tmp_cap.ParseCapacitor(tokens, syntax_match, this->_nodes, this->_base_element_names, this->_caps.size());
                this->_caps.push_back(tmp_cap);

                break;
            }
            case 'L':
            {
                Coil tmp_coil;
                errcode = tmp_coil.ParseCoil(tokens, syntax_match, this->_nodes, this->_base_element_names, this->_coils.size());
                this->_coils.push_back(tmp_coil);

                break;
            }
            case 'I':
            {
                ics tmp_ics;
                errcode = tmp_ics.ParseIcs(tokens, syntax_match, this->_nodes, this->_base_element_names, this->_ics.size());
                this->_ics.push_back(tmp_ics);

                break;
            }
            case 'V':
            {
                ivs tmp_ivs;
                errcode = tmp_ivs.ParseIvs(tokens, syntax_match, this->_nodes, this->_base_element_names, this->_ivs.size());
                this->_ivs.push_back(tmp_ivs);

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
    auto end = chrono::high_resolution_clock::now();

    /* Output information only in case of success */
    if(errcode == RETURN_SUCCESS)
    {
		cout << "************************************" << endl;
		cout << "************CIRCUIT INFO************" << endl;
		cout << "************************************" << endl;
		cout << "Load time: " << chrono::duration_cast<chrono::milliseconds>(end-begin).count() << "ms" << endl;
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
		cout << "Total simulation points: " << this->_sim_vals.size() << endl;
		cout << "Total nodes to plot: " << this->_plot_nodes.size() << endl;
		cout << "Total sources to plot: " << this->_plot_sources.size() << endl;
		cout << "************************************" << endl;
    }

    return errcode;
}

/*!
    @brief    Internal routine, that parses and forms a SPICE card given the syntax matcher
    and the tokens of the element.
  	  - Plot nodes for a plot card already exists in the circuit
  	  - DC analysis source already exists in the circuit (if any DC analysis is active)
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
		if(match.isValidSetPLOTCard(tokens, this->_plot_nodes, this->_plot_sources))
			return RETURN_SUCCESS;
		else
			return FAIL_PARSER_INVALID_FORMAT;
	}

	/* Base parameters for analysis */
	double start, stop, steps;

	/* Before moving to an analysis case, clear any existing simulation points */
	this->_sim_vals.clear();

	/* Handle each analaysis */
	if(spice_card == "OP" && tokens.size() == 1)
	{
		this->_type = OP;
		return RETURN_SUCCESS;
	}
	else if(spice_card == "DC")
	{
		if(!match.isValidDCCard(tokens, steps, stop, start, this->_scale, this->_source))
			return FAIL_PARSER_INVALID_FORMAT;

		/* Timing checks */
		if((stop <= start) || (steps <= 0))
			return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

		this->_type = DC;
	}
	else if(spice_card == "TRAN")
	{
		if(!match.isValidTRANCard(tokens, steps, stop, start))
			return FAIL_PARSER_INVALID_FORMAT;

		/* Timing checks */
		if((stop <= start) || (steps <= 0) || (start < 0.0))
			return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

		this->_type = TRAN;
	}
	else if(spice_card == "AC")
	{
		if(!match.isValidACCard(tokens, steps, stop, start, this->_scale))
			return FAIL_PARSER_INVALID_FORMAT;

		/* Timing checks */
		if((stop <= start) || (steps <= 0) || (start <= 0.0))
			return FAIL_PARSER_ANALYSIS_INVALID_ARGS;

		this->_type = AC;
	}
	else
	{
		return FAIL_PARSER_UKNOWN_SPICE_CARD;
	}

	/* Now create the simulation times/values vector */
	if(this->_scale == DEC_SCALE)
	{
		/* We need to exclude <0> timepoint in case of transient analysis */
		if(start == 0.0 && this->_type == TRAN) start += steps;

		/* For AC we generate based on step (since argument is [points]) */
		if(this->_type == AC) linspaceVecGen(this->_sim_vals, start, stop, static_cast<size_t>(steps));
		else StepVecGen(this->_sim_vals, start, stop, steps);
	}
	else
	{
		/* Generate logarithmically spaced vector */
		logspaceVecGen(this->_sim_vals, start, stop, static_cast<size_t>(steps));
	}

    return RETURN_SUCCESS;
}

/*!
    @brief    Internal routine, that verifies the following for a given circuit:
  	  - Plot nodes for a plot card already exists in the circuit
  	  - DC analysis source already exists in the circuit (if any DC analysis is active)
    @return   The error code, in case of error, otherwise RETURN_SUCCESS.
*/
return_codes_e Circuit::verify(void)
{
	using namespace std;

	return_codes_e errcode;
    auto namemap_end = this->_base_element_names.end();
    auto nodemap_end = this->_nodes.end();

    /* After parsing the file, in case of DC analysis verify that the simulated source exists */
    if(this->_type == DC)
    {
    	auto name_it = this->_base_element_names.find(this->_source);

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
    	auto node_it = this->_base_element_names.find(*it);

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

    return RETURN_SUCCESS;
}
