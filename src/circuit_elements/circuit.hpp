#ifndef __CIRCUIT_H
#define __CIRCUIT_H

#include <unordered_map>
#include <iostream>
#include <fstream>
#include "syntax_parser.hpp"
#include "circuit_elements.hpp"

class Circuit
{
    public:

		/*!
			@brief The default constructor of the class.
		*/
		Circuit()
		{
			this->_scale = DEC_SCALE;
			this->_type = OP;
		}

        /*!
            @brief    Get the resistors in the circuit.
            @return   The resistors vector.
        */
        std::vector<Resistor> &getResistors(void)
        {
            return _res;
        }

        /*!
            @brief    Get the capacitors in the circuit.
            @return   The capacitors vector.
        */
        std::vector<Capacitor> &getCapacitors(void)
        {
            return _caps;
        }

        /*!
            @brief    Get the coils in the circuit.
            @return   The coils vector.
        */
        std::vector<Coil> &getCoils(void)
        {
            return _coils;
        }

        /*!
            @brief    Get the ICS in the circuit.
            @return   The ICS vector.
        */
        std::vector<ics> &getICS(void)
        {
            return _ics;
        }

        /*!
            @brief    Get the IVS in the circuit.
            @return   The IVS vector.
        */
        std::vector<ivs> &getIVS(void)
        {
            return _ivs;
        }

        /*!
            @brief    Get the nodes map (map contains the <NodeName, NodeNum> pairs).
            @return   The map.
        */
        std::unordered_map<std::string, long int> &getNodes(void)
        {
            return _nodes;
        }

        /*!
            @brief    Get the elements map (map contains the <ElmementName, ElementID> pairs).
            @return   The map.
        */
        std::unordered_map<std::string, size_t> &getElementNames(void)
        {
            return _base_element_names;
        }

        /*!
            @brief    Get the nodes to be plotted in the circuit.
            @return   The nodesNames vector.
        */
        std::vector<std::string> &getPlotNodes(void)
        {
            return _plot_nodes;
        }

        /*!
            @brief    Get the sources to be plotted in the circuit.
            @return   The sources vector.
        */
        std::vector<std::string> &getPlotSources(void)
        {
            return _plot_sources;
        }


        return_codes_e CreateCircuit(std::fstream &input_file);
    private:
        return_codes_e CreateSPICECard(std::vector<std::string> &tokens, syntax_parser &match);
        return_codes_e verify(void);

        /* A circuit contains the elements supported by the simulator - Ordered by name inside each container */
        std::vector<Resistor> _res;
        std::vector<Capacitor> _caps;
        std::vector<Coil> _coils;
        std::vector<ics> _ics;
        std::vector<ivs> _ivs;

        /* Elements map - Nodes map */
        std::unordered_map<std::string, size_t> _base_element_names;
        std::unordered_map<std::string, long int> _nodes;

        /* SPICE CARDS - Analysis */
        std::vector<double> _sim_vals; 	/* The simulation points to be used */
        as_scale_t _scale;				/* Scale of the analysis */
        analysis_t _type;				/* Analysis type */
        std::string _source;			/* In case of DC analysis - Name of source */

        /* SPICE CARDS - Plot */
        std::vector<std::string> _plot_nodes;
        std::vector<std::string> _plot_sources;
};

#endif // __CIRCUIT_H //
