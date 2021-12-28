#ifndef __ICS_HPP
#define __ICS_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include "simulator_codes.hpp"
#include "syntax_parser.hpp"

class ics
{
    public:
        /*!
            @brief    Default constructor.
        */
        ics(void)
        {
            _current_value = 0;
            _ac_mag = 0;
            _ac_phase = 0;
            _type = CONSTANT;
        }

        /*!
            @brief    Get the current value
            @return   The value.
        */
        double getVal(void)
        {
            return _current_value;
        }

        /*!
            @brief    Get the name of this current source (I is included).
            @return   The name.
        */
        std::string &getName(void)
        {
            return _name;
        }

        /*!
            @brief    Get the attached nodes of the current source, in the form of <nodeName, nodeNum>.
            @return   The array of the node tuples.
        */
        std::array<std::tuple<std::string, long int>, 2> &getNodes(void)
        {
            return _nodes;
        }

        /*!
            @brief      Routine parses the token stream and checks the input maps where it:
                            - Verifies correct grammar and syntax of the tokens.
                            - Verifies the uniqueness of the element.
                            - Checks if new nodes are added to the circuit.
            The values then are loaded in this element, therefore this routine serving as the main initializer
            of this type of element. A current source is an element of this type (2-node extend-2):
                                    ##### I<name> <V+> <V-> <Value> {AC <mag> <phase>} {TRAN_SPEC} #####

            @param      tokens    The tokens that form the current source element.
            @param      match     Syntax parser instance.
            @param      nodes     Map that contains all the nodes in the circuit along with their unique nodeNum.
            @param      elements  Map that contains all the unique elements along with their ID in the circuit.
            @param      ics_id    Unique ID of this current source.
            @return     RETURN_SUCCESS or appropriate failure code.
        */
        return_codes_e ParseIcs(std::vector<std::string> &tokens,
                                 syntax_parser &match,
                                 std::unordered_map<std::string, long int> &nodes,
                                 std::unordered_map<std::string, size_t> &elements,
                                 size_t ics_id)
        {
            /* Check correct syntax */
            if(!match.isValidSource(tokens, this->_current_value, this->_ac_mag, this->_ac_phase, this->_tran_time, this->_tran_vals, this->_type))
                return FAIL_PARSER_INVALID_FORMAT;

            /* Check correct grammar */
            if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
            elements[tokens[0]] = ics_id;

            /* Check that nodes are different */
            if(tokens[1] == tokens[2]) return FAIL_PARSER_SHORTED_ELEMENT;

            /* Name of the device */
            this->_name = tokens[0];

            /* Check for the nodes if they already exist and valid connectivity (shorts) */
            for(int i = 0; i < 2; i++)
            {
                /* Ground node - Do not insert in map (-1) */
                if(tokens[i + 1] == "0")
                {
                    this->_nodes[i] = std::make_tuple("0", -1);
                    continue;
                }

                /* Normal node */
                auto it = nodes.find(tokens[i + 1]);

                if(it != nodes.end()) /* First time encountering this node */
                {
                    this->_nodes[i] = std::make_tuple(it->first, it->second);
                }
                else
                {
                    size_t tmp_sz = nodes.size();
                    this->_nodes[i] = std::make_tuple(tokens[i + 1], tmp_sz);
                    nodes[tokens[i + 1]] = tmp_sz; /* Also insert in map */
                }
            }

            return RETURN_SUCCESS;
        }

    private:
        std::string _name;                                          /* Name in the form of Cxxx */
        double _current_value;                                      /* Value in Amperes */
        std::array<std::tuple<std::string, long int>, 2> _nodes;    /* The combination of the node names */

        /* AC analysis components - Optional */
        double _ac_mag, _ac_phase;

        /* TRAN analysis components - Optional*/
        std::vector<double> _tran_vals;
        std::vector<double> _tran_time;
        tran_source_t _type;
};

#endif // __ICS_HPP //
