#ifndef __IVS_HPP
#define __IVS_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "simulator_types.hpp"
#include "syntax_parser.hpp"

class ivs
{
    public:
        /*!
            @brief    Default constructor.
        */
        ivs(void)
        {
            _voltage_value = 0;
            _ac_mag = 0;
            _ac_phase = 0;
            _type = CONSTANT;
        }

        /*!
            @brief    Get the voltage value
            @return   The value.
        */
        double getVal(void)
        {
            return _voltage_value;
        }

        /*!
            @brief    Get the name of this voltage source (V is included).
            @return   The name.
        */
        std::string &getName(void)
        {
            return _name;
        }

        /*!
            @brief    Get the attached nodes of the voltage source, in the form of <nodeName, nodeNum>.
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
            of this type of element. A voltage source is an element of this type (2-node extend-2):
                                    ##### V<name> <V+> <V-> <Value> {AC <mag> <phase>} {TRAN_SPEC} #####

            @param      tokens    The tokens that form the voltage source element.
            @param      match     Syntax parser instance.
            @param      nodes     Map that contains all the nodes in the circuit along with their unique nodeNum.
            @param      elements  Map that contains all the unique elements along with their ID in the circuit.
            @param      ivs_id    Unique ID of this voltage source.
            @return     RETURN_SUCCESS or appropriate failure code.
        */
        return_codes_e ParseIvs(std::vector<std::string> &tokens,
                                 syntax_parser &match,
                                 std::unordered_map<std::string, long int> &nodes,
                                 std::unordered_map<std::string, size_t> &elements,
                                 size_t ivs_id)
        {
            using namespace std;

            /* Check correct syntax */
            if(!match.isValidSource(tokens, this->_voltage_value, this->_ac_mag, this->_ac_phase, this->_tran_time, this->_tran_vals, this->_type))
                return FAIL_PARSER_INVALID_FORMAT;

            /* Check correct grammar */
            if(elements.find(tokens[0]) != elements.end()) return FAIL_PARSER_ELEMENT_EXISTS;
            elements[tokens[0]] = ivs_id;

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
            @brief      Inserts the MNA stamp of the IVS in triplet form
            (i, j, val) inside the mat array. IVS need to be inserted
            in the sub-arrays for sources, hence the offset.
            For DC analysis coils have 3 or 5 stamps.
            @param      mat    The triplet matrix to insert the stamp.
            @param		offset The offset of the stamp inside the array
        */
        void MNAStampDC(tripletList &mat, DensVecD &rh, long int offset)
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

			rh[offset] += this->_voltage_value;
        }

    private:
        std::string _name;                                       /* Name in the form of Vxxx */
        double _voltage_value;                                   /* Value in Volts */
        std::array<std::tuple<std::string, long int>, 2> _nodes; /* The combination of the node names */

        /* AC analysis components - Optional */
        double _ac_mag, _ac_phase;

        /* TRAN analysis components - Optional*/
        std::vector<double> _tran_vals;
        std::vector<double> _tran_time;
        tran_source_t _type;
};

#endif // __IVS_HPP //
