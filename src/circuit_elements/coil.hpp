#ifndef __COIL_HPP
#define __COIL_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "simulator_types.hpp"
#include "syntax_parser.hpp"

class Coil
{
    public:
        /*!
            @brief    Default constructor.
        */
        Coil(void)
        {
            _inductance = 0;
        }

        /*!
            @brief    Get the inductance value.
            @return   The value.
        */
        double getVal(void)
        {
            return _inductance;
        }

        /*!
            @brief    Get the name of this coil (L is included).
            @return   The name.
        */
        std::string &getName(void)
        {
            return _name;
        }

        /*!
            @brief    Get the attached nodes of the coil, in the form of <nodeName, nodeNum>.
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
            of this type of element. A coil is an element of this type (2-node basic):
                                    ##### L<name> <V+> <V-> <Value> #####

            @param      tokens    The tokens that form the coil element.
            @param      match     Syntax parser instance.
            @param      nodes     Map that contains all the nodes in the circuit along with their unique nodeNum.
            @param      elements  Map that contains all the unique elements along with their ID in the circuit.
            @param      coil_id   Unique ID of this coil.
            @return     RETURN_SUCCESS or appropriate failure code.
        */
        return_codes_e ParseCoil(std::vector<std::string> &tokens,
                                 syntax_parser &match,
                                 std::unordered_map<std::string, long int> &nodes,
                                 std::unordered_map<std::string, size_t> &elements,
                                 size_t coil_id)
        {
            /* Check correct syntax */
            if(!match.isValidTwoNodeElement(tokens, this->_inductance)) return FAIL_PARSER_INVALID_FORMAT;

            if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
            elements[tokens[0]] = coil_id;

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
            @brief      Inserts the MNA stamp of the coil in triplet form
            (i, j, val) inside the mat array. Since coils act like DC shorts
            they need to be inserted in the sub-arrays for sources, hence the
            offset. For DC analysis coils have 2 or 4 stamps.
            @param      mat    The triplet matrix to insert the stamp.
            @param		offset The offset of the stamp inside the array
        */
        void MNAStampDC(tripletList &mat, long int offset)
        {
        	long int pos = std::get<1>(this->_nodes[0]);
        	long int neg = std::get<1>(this->_nodes[1]);

			if(pos != -1)
			{
				mat.push_back(triplet_eig(offset, pos, 1));
				mat.push_back(triplet_eig(pos, offset, 1));
			}

			if(neg != -1)
			{
				mat.push_back(triplet_eig(offset, neg, -1));
				mat.push_back(triplet_eig(neg, offset, -1));
			}
        }

    private:
        std::string _name;                                      /* Name in the form of Lxxx */
        double _inductance;                                     /* Value in Henrys */
        std::array<std::tuple<std::string, long int>, 2> _nodes;  /* The combination of the node names */
};

#endif // __COIL_HPP //
