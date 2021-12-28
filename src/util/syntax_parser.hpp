#ifndef __PARSER_REGEX_H
#define __PARSER_REGEX_H

#include <string>
#include <regex>
#include <vector>

#include "../simulator_types.hpp"

class syntax_parser
{
    public:
		/* Spice line tokenizer */
		bool tokenizer(std::string &line, std::vector<std::string> &tokens);

        /* Spice Elements */
        bool isValidTwoNodeElement(std::vector<std::string> &tokens, double &converted_val);
        bool isValidFourNodeElement(std::vector<std::string> &tokens, double &converted_val);
        bool isValidCurrentControlElement(std::vector<std::string> &tokens, double &converted_val);
        bool isValidSource(std::vector<std::string> &tokens, double &converted_val, double &ac_mag,
                           double &ac_phase, std::vector<double> &vvals, std::vector<double> &tvals, tran_source_t source);

        /* Spice Cards */
        bool isValidTRANCard(std::vector<std::string> &tokens, double &step, double &tstop, double &tstart);
        bool isValidACCard(std::vector<std::string> &tokens, double &points, double &fstop, double &fstart, as_scale_t &scale);
        bool isValidDCCard(std::vector<std::string> &tokens, double &points, double &stop, double &start, as_scale_t &scale, std::string &source);
        bool isValidSetPLOTCard(std::vector<std::string> &tokens, std::vector<std::string> &plot_nodes, std::vector<std::string> &plot_sources);

    private:
        /* Internal methods for components */
        bool IsValidNode(const std::string &node);
        bool IsValidName(const std::string &node);
        bool IsValidValue(std::string &node);
        bool IsValidSetValue(std::string &node, double &val);
        bool IsValidSetValueInt(std::string &node, double &val);

        // TODO - Also modify to have modifiers for the floating number
//        _decimal_number = std::regex("(([0-9]+)(\\.[0-9]+)?([FPNUMKGT]|(MEG))?)|([0-9]+E[+-][0-9]+)", std::regex_constants::icase);

        // TODO - Choose the version either strtok or stdregex
//        const std::regex _delimiters = std::regex("[ (),\\s]+");      /* Delimiters */
        const char* _delimiters = " (),\t\n\r";
        const std::regex _alphanumeric = std::regex("[[:alnum:]]+", std::regex_constants::icase);
        const std::regex _decimal_number = std::regex("([[:digit:]]+)(\\.[[:digit:]]+)?(E[+-][[:digit:]]+)?", std::regex_constants::icase);
        const std::regex _integer_number = std::regex("([[:digit:]]+)(E[+-][[:digit:]]+)?", std::regex_constants::icase);
        const std::regex _alphanumeric_with_underscores = std::regex("[[:alnum:]_]+", std::regex_constants::icase);
};

#endif // __PARSER_REGEX_H //
