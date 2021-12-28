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
			this->_valid = false;
		}

        /*!
            @brief    Cleans up the circuit by reseting to default state.
            The data structures are also resized to avoid potential
            memory waste.
        */
		void clear(void)
		{
			/* Default */
			this->_scale = DEC_SCALE;
			this->_type = OP;
			this->_source = "";
			this->_valid = false;

			/* For vectors call clean method to accelerate next circuit */
			this->_res.clear();
			this->_caps.clear();
			this->_coils.clear();
			this->_ics.clear();
			this->_ivs.clear();
			this->_sim_vals.clear();
			this->_plot_nodes.clear();
			this->_plot_sources.clear();

			/* Maps */
			this->_element_names.clear();
			this->_nodes.clear();
		}

        /*!
            @brief    Check if the current circuit is valid.
            @return   True or False.
        */
        bool isCircuitValid(void)
        {
            return _valid;
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
            return _element_names;
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

        /*!
            @brief    Get the simulation values for the circuit.
            @return   The simulation vector (either in A/V, s, Hz).
        */
        std::vector<double> &getSimValues(void)
        {
            return _sim_vals;
        }

        /*!
            @brief    Get the DC source for analysis.
            @return   The DC source name.
        */
        std::string &getDCSource(void)
        {
            return _source;
        }

        /*!
            @brief    Get the analysis type.
            @return   The type enum.
        */
        analysis_t getAnalysisType(void)
        {
        	return this->_type;
        }

        /*!
            @brief    Get the analysis scale.
            @return   The scale enum.
        */
        as_scale_t getAnalysisScale(void)
        {
        	return this->_scale;
        }

        return_codes_e CreateCircuit(std::fstream &input_file);
    private:
        return_codes_e CreateSPICECard(std::vector<std::string> &tokens, syntax_parser &match);
        return_codes_e verify(void);

        /* Flag */
        bool _valid;

        /* A circuit contains the elements supported by the simulator */
        std::vector<Resistor> _res;
        std::vector<Capacitor> _caps;
        std::vector<Coil> _coils;
        std::vector<ics> _ics;
        std::vector<ivs> _ivs;

        /* Elements/Nodes maps */
        std::unordered_map<std::string, size_t> _element_names;
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
