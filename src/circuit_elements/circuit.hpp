#ifndef __CIRCUIT_H
#define __CIRCUIT_H

#include <iostream>
#include <fstream>
#include "parser.hpp"

class Circuit
{
    public:

		/*!
			@brief The default constructor of the class.
		*/
		Circuit()
		{
		    this->_ode_method = BACKWARDS_EULER;
			this->_scale = DEC_SCALE;
			this->_type = OP;
			this->_mem_save_mode = false;
			this->_valid = false;
			this->_sim_step = 0;
			this->_sim_start = 0;
			this->_sim_end = 0;
		}

        /*!
            @brief    Cleans up the circuit by reseting to default state.
            The data structures are also resized to avoid potential
            memory waste.
        */
		void clear(void)
		{
			/* Default */
		    this->_ode_method = BACKWARDS_EULER;
			this->_scale = DEC_SCALE;
			this->_type = OP;
			this->_mem_save_mode = false;
			this->_valid = false;
			this->_source = "";
			this->_sim_step = 0;
			this->_sim_start = 1;
			this->_sim_end = 1;

			/* For vectors call clean method to accelerate next circuit */
			this->_res.clear();
			this->_caps.clear();
			this->_coils.clear();
			this->_ics.clear();
			this->_ivs.clear();
            this->_vccs.clear();
            this->_vcvs.clear();
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
        bool valid(void)
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
            @brief    Get the VCVS in the circuit.
            @return   The VCVS vector.
        */
        std::vector<vcvs> &getVCVS(void)
        {
            return _vcvs;
        }

        /*!
            @brief    Get the VCCS in the circuit.
            @return   The VCCS vector.
        */
        std::vector<vccs> &getVCCS(void)
        {
            return _vccs;
        }

        /*!
            @brief    Get the nodes map (map contains the <NodeName, NodeNum> pairs).
            @return   The map.
        */
        hashmap_str_t &getNodes(void)
        {
            return _nodes;
        }

        /*!
            @brief    Get the elements map (map contains the <ElmementName, ElementID> pairs).
            @return   The map.
        */
        hashmap_str_t &getElementNames(void)
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
            @brief    Get the simulation start value.
            @return   The start.
        */
        double &getSimStart(void)
        {
            return _sim_start;
        }

        /*!
            @brief    Get the simulation end value.
            @return   The end.
        */
        double &getSimEnd(void)
        {
            return _sim_end;
        }

        /*!
            @brief    Get the simulation step.
            @return   The step.
        */
        double getSimStep(void)
        {
        	return this->_sim_step;
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

        /*!
            @brief    Get the ODE method to be used for transient.
            @return   The method.
        */
        ODE_meth_t getODEMethod(void)
        {
            return this->_ode_method;
        }

        /*!
            @brief    Get the memory mode the simulation is going to be run at.
            @return   Memory save(true) otherwise store all results (false).
        */
        bool getMemMode(void)
        {
            return this->_mem_save_mode;
        }

        return_codes_e create(std::string &input_file_name);
    private:
        return_codes_e setCircuitOptions(std::vector<std::string> &tokens);
        return_codes_e createSPICECard(std::vector<std::string> &tokens, parser &match);
        return_codes_e verify(void);

        /* Debugging only functions */
        void debug_insert_nodes(node2_device &element);
        void debug_circuit(void);

        /* Flag indicating this is a valid circuit */
        bool _valid;

        /* Elements contained in the circuit */
        std::vector<Resistor> _res;
        std::vector<Capacitor> _caps;
        std::vector<Coil> _coils;
        std::vector<ics> _ics;
        std::vector<ivs> _ivs;
        std::vector<vcvs> _vcvs;
        std::vector<vccs> _vccs;

        /* Elements/Nodes maps */
        hashmap_str_t _element_names;
        hashmap_str_t _nodes;

        /* SPICE CARDS/OPTIONS - Analysis */
        double _sim_start;				/* The simulation start value */
        double _sim_end;				/* The simulation end value */
        double _sim_step;				/* The simulation step */
        as_scale_t _scale;				/* Scale of the analysis */
        analysis_t _type;				/* Analysis type */
        ODE_meth_t _ode_method;         /* ODE method in case of transient */
        bool _mem_save_mode;            /* Run analysis in memory save mode */
        std::string _source;			/* In case of DC analysis - Name of source */

        /* SPICE CARDS - Plot */
        std::vector<std::string> _plot_nodes;
        std::vector<std::string> _plot_sources;
};

#endif // __CIRCUIT_H //
