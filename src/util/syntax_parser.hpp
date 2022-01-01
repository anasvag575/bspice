#ifndef __PARSER_REGEX_H
#define __PARSER_REGEX_H

#include <string>
#include <regex>
#include <vector>

#include "base_types.hpp"
#include "circuit_elements.hpp"
#include "simulator_types.hpp"

class syntax_parser
{
    public:
		/* Spice line tokenizer */
		bool tokenizer(std::string &line, std::vector<std::string> &tokens);

		/* Spice elements */
		return_codes_e Parse2NodeDevice(std::vector<std::string> &tokens,
									    node2_device &element,
									    hashmap_str_t &nodes,
									    hashmap_str_t &elements,
									    size_t device_id,
									    bool complete);

		return_codes_e ParseSourceSpec(std::vector<std::string> &tokens, source_spec &spec);

        /* Spice Cards */
		return_codes_e isValidDCCard(std::vector<std::string> &tokens, double &points, double &stop, double &start, as_scale_t &scale, std::string &source);
		return_codes_e isValidTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart);
		return_codes_e isValidACCard(std::vector<std::string> &tokens, double &points, double &fstop, double &fstart, as_scale_t &scale);
		return_codes_e isValidSetPLOTCard(std::vector<std::string> &tokens, std::vector<std::string> &plot_nodes, std::vector<std::string> &plot_sources);

    private:
        /* Methods for components */
        bool IsValidNode(const std::string &node);
        bool IsValidName(const std::string &node);
        bool IsValidValue(std::string &node);
        bool IsValidSetValue(std::string &node, double &val);
        bool IsValidSetValueInt(std::string &node, double &val);

        /* Methods for Spice Elements */
        bool isValidTwoNodeElement(std::vector<std::string> &tokens, double &converted_val);
        bool isValidFourNodeElement(std::vector<std::string> &tokens, double &converted_val);
        bool isValidCurrentControlElement(std::vector<std::string> &tokens, double &converted_val);

        // TODO - Also modify to have modifiers for the floating number extensions
//        _decimal_number = std::regex("(([0-9]+)(\\.[0-9]+)?([FPNUMKGT]|(MEG))?)|([0-9]+E[+-][0-9]+)", std::regex_constants::icase);

        /* Delimiters */	// TODO - Choose the version either strtok or stdregex
//        const std::regex _delimiters = std::regex("[ (),\\s]+");
        const char* _delimiters = " (),\t\n\r";
        const std::regex _alphanumeric = std::regex("[[:alnum:]]+", std::regex_constants::icase);
        const std::regex _decimal_number = std::regex("([[:digit:]]+)(\\.[[:digit:]]+)?(E[+-][[:digit:]]+)?", std::regex_constants::icase);
        const std::regex _integer_number = std::regex("([[:digit:]]+)(E[+-][[:digit:]]+)?", std::regex_constants::icase);
        const std::regex _alphanumeric_with_underscores = std::regex("[[:alnum:]_]+", std::regex_constants::icase);
};

#endif // __PARSER_REGEX_H //
