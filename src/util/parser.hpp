#ifndef __PARSER_H
#define __PARSER_H

#include <regex>
#include "base_types.hpp"
#include "circuit_elements.hpp"
#include "simulator_types.hpp"

/**
    Class that defines a SPICE parser engine.
    Provides a tokenizer, converters for strings to numbers and contaings
    the basic parsing routines for all the SPICE elements and cards.
 */
class parser
{
    public:
		/* Spice line tokenizer */
		bool tokenizer(std::string &line, std::vector<std::string> &tokens);

		/* Spice elements */
		return_codes_e parse2NodeDevice(std::vector<std::string> &tokens,
									    node2_device &element,
									    hashmap_str_t &elements,
										hashmap_str_t &nodes,
									    size_t device_id,
									    bool complete);

        return_codes_e parse2SNodeDevice(std::vector<std::string> &tokens,
                                        node2s_device &element,
                                        hashmap_str_t &elements,
                                        hashmap_str_t &nodes,
                                        size_t device_id);

        return_codes_e parse4NodeDevice(std::vector<std::string> &tokens,
                                        node4_device &element,
                                        hashmap_str_t &elements,
                                        hashmap_str_t &nodes,
                                        size_t device_id);

		return_codes_e parseSourceSpec(std::vector<std::string> &tokens, source_spec &spec);

        /* Spice Cards */
		return_codes_e parseDCCard(std::vector<std::string> &tokens, double &points, double &stop, double &start, as_scale_t &scale, std::string &source);
		return_codes_e parseTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart);
		return_codes_e parseACCard(std::vector<std::string> &tokens, double &points, double &fstop, double &fstart, as_scale_t &scale);
		return_codes_e parsePLOTCard(std::vector<std::string> &tokens, std::vector<std::string> &plot_nodes, std::vector<std::string> &plot_sources);

    private:
		/* Grammar methods for components */
		IntTp resolveNodeID(hashmap_str_t &nodes, const std::string &node);
		double resolveFloatNum(const std::string &num);
		IntTp resolveIntNum(const std::string &num);

        /* Syntax methods for components */
        bool IsValidNode(const std::string &token);
        bool IsValidName(const std::string &token);
        bool IsValidFpValue(const std::string &token);
        bool IsValidIntValue(std::string &token);

        /* Methods for Spice Elements */
        bool isValidTwoNodeElement(std::vector<std::string> &tokens);
        bool isValidTwoNodeEnhancedElement(std::vector<std::string> &tokens);
        bool isValidFourNodeElement(std::vector<std::string> &tokens);
        bool isValidCurrentControlElement(std::vector<std::string> &tokens);

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

#endif // __SYNTAX_PARSER_H //
