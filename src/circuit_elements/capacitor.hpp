#ifndef __CAPACITOR_HPP
#define __CAPACITOR_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "simulator_types.hpp"
#include "syntax_parser.hpp"

class Capacitor
{
    public:
        /*!
            @brief    Default constructor.
        */
        Capacitor(void)
        {
            _capacitance = 0;
        }

        /*!
            @brief    Get the capacitance value
            @return   The value.
        */
        double getVal(void)
        {
            return _capacitance;
        }

        /*!
            @brief    Get the name of this capacitor (C is included).
            @return   The name.
        */
        std::string &getName(void)
        {
            return _name;
        }

        /*!
            @brief    Get the attached nodes of the capacitor, in the form of <nodeName, nodeNum>.
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
            of this type of element. A capacitor is an element of this type (2-node basic):
                                    ##### C<name> <V+> <V-> <Value> #####

            @param      tokens    The tokens that form the capacitor element.
            @param      match     Syntax parser instance.
            @param      nodes     Map that contains all the nodes in the circuit along with their unique nodeNum.
            @param      elements  Map that contains all the unique elements along with their ID in the circuit.
            @param      cap_id    Unique ID of this capacitor.
            @return     RETURN_SUCCESS or appropriate failure code.
        */
        return_codes_e ParseCapacitor(std::vector<std::string> &tokens,
                                      syntax_parser &match,
                                      std::unordered_map<std::string, long int> &nodes,
                                      std::unordered_map<std::string, size_t> &elements,
                                      size_t cap_id)
        {
            /* Check correct syntax */
            if(!match.isValidTwoNodeElement(tokens, this->_capacitance)) return FAIL_PARSER_INVALID_FORMAT;

            /* Element has to be unique */
            if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
            elements[tokens[0]] = cap_id;

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

        /*!
            @brief      Inserts the MNA stamp of the capacitor in triplet form
            (i, j, val) inside the mat array. For DC analysis capacitors have 0 stamps.
            @param      mat    The triplet matrix to insert the stamp.
        */
        void MNAStampDC(tripletList &mat){;}

    private:
        std::string _name;                                      /* Name in the form of Cxxx */
        double _capacitance;                                    /* Value in Fahrads */
        std::array<std::tuple<std::string, long int>, 2> _nodes;  /* Device attaching <NodeName, NodeNum> pairs */
};

#endif // __CAPACITOR_HPP //
