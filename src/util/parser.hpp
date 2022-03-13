#ifndef __PARSER_H
#define __PARSER_H

#include <regex>
#include "base_types.hpp"
#include "circuit_elements.hpp"
#include "simulator_types.hpp"



//! A parser class. The purpose of this class is to provide a SPICE parser engine.
/*!
  The class tokenizes lines of the SPICE netlist, which then form SPICE elements
  or cards. These are resolved using the provided syntax/grammar methods.
*/
class parser
{
    public:
		/* Spice line tokenizer */
		bool tokenizer(const std::string &line, std::vector<std::string> &tokens);

		/* Spice elements */
		return_codes_e parse2NodeDevice(const std::vector<std::string> &tokens,
									    node2_device &element,
									    hashmap_str_t &elements,
										hashmap_str_t &nodes,
									    size_t device_id,
									    bool complete);

        return_codes_e parse2SNodeDevice(const std::vector<std::string> &tokens,
                                        node2s_device &element,
                                        hashmap_str_t &elements,
                                        hashmap_str_t &nodes,
                                        size_t device_id);

        return_codes_e parse4NodeDevice(const std::vector<std::string> &tokens,
                                        node4_device &element,
                                        hashmap_str_t &elements,
                                        hashmap_str_t &nodes,
                                        size_t device_id);

		return_codes_e parseSourceSpec(const std::vector<std::string> &tokens, source_spec &spec);

        /* Spice Cards */
		return_codes_e parseDCCard(const std::vector<std::string> &tokens,
		                           double &points,
		                           double &stop,
		                           double &start,
		                           as_scale_t &scale,
		                           std::string &source);

		return_codes_e parseTRANCard(const std::vector<std::string> &tokens,
		                             double &step,
		                             double &tstop,
		                             double &tstart);

		return_codes_e parseACCard(const std::vector<std::string> &tokens,
		                           double &points,
		                           double &fstop,
		                           double &fstart,
		                           as_scale_t &scale);

		return_codes_e parsePLOTCard(const std::vector<std::string> &tokens,
		                             std::vector<std::string> &plot_nodes,
		                             std::vector<std::string> &plot_sources);

    private:
		/* Grammar methods for components */
		IntTp resolveNodeID(hashmap_str_t &nodes, const std::string &node);
		double resolveFloatNum(const std::string &num);
		IntTp resolveIntNum(const std::string &num);

        /* Syntax methods for components */
        bool IsValidNode(const std::string &token);
        bool IsValidName(const std::string &token);
        bool IsValidFpValue(const std::string &token);
        bool IsValidIntValue(const std::string &token);

        /* Methods for Spice Elements */
        bool isValidTwoNodeElement(const std::vector<std::string> &tokens);
        bool isValidFourNodeElement(const std::vector<std::string> &tokens);
        bool isValidCurrentControlElement(const std::vector<std::string> &tokens);

        // TODO - Also modify to have modifiers for the floating number extensions
//        _decimal_number = std::regex("(([0-9]+)(\\.[0-9]+)?([FPNUMKGT]|(MEG))?)|([0-9]+E[+-][0-9]+)", std::regex_constants::icase);

#ifdef DBSPICE_TOKENIZER_USE_REGEX
        /** Regex with all the delimiter characters used for tokenization. */
        const std::regex _delimiters = std::regex("[ (),\\s]+");
#else
        /** String with all the delimiter characters used for tokenization. */
        const char* _delimiters = " (),\t\n\r";
#endif
        /** Regex pattern used for alphanumeric types. */
        const std::regex _alphanumeric = std::regex("[[:alnum:]]+", std::regex_constants::icase);

        /** Regex pattern used for decimal numbers. */
        const std::regex _decimal_number = std::regex("([[:digit:]]+)(\\.[[:digit:]]+)?(E[+-][[:digit:]]+)?", std::regex_constants::icase);

        /** Regex pattern used for integer numbers. */
        const std::regex _integer_number = std::regex("([[:digit:]]+)(E[+-][[:digit:]]+)?", std::regex_constants::icase);

        /** Regex pattern used for alphanumerics with underscores. */
        const std::regex _alphanumeric_with_underscores = std::regex("[[:alnum:]_]+", std::regex_constants::icase);
};

#endif // __PARSER_H //
