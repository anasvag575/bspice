#ifndef __CIRCUIT_H
#define __CIRCUIT_H

#include <iostream>
#include <fstream>
#include "parser.hpp"

//! A circuit class. The purpose of this class is to represent a SPICE netlist.
/*!
  This class has all the information provided from the SPICE netlist. This information
  includes:
  - Simulator options.
  - SPICE Elements with their respective connections.
  - SPICE cards and their information.
  - Information for assisting the BSPICE operation.
*/
class circuit
{
    public:
        /* Constructors */
		circuit(const std::string &input_file_name);

        /* Elements contained in the circuit */
        const std::vector<resistor> &Resistors(void);
        const std::vector<capacitor> &Capacitors(void);
        const std::vector<coil> &Coils(void);
        const std::vector<ics> &ICS(void);
        const std::vector<ivs> &IVS(void);
        const std::vector<vcvs> &VCVS(void);
        const std::vector<vccs> &VCCS(void);
        const std::vector<ccvs> &CCVS(void);
        const std::vector<cccs> &CCCS(void);

        /* Elements/Nodes/Plot names */
        const hashmap_str_t &Nodes(void);
        const hashmap_str_t &ElementNames(void);
        const std::vector<std::string> &PlotNodes(void);
        const std::vector<std::string> &PlotSources(void);
        const std::string &DCSource(void);

        /* Analysis specifics */
        double SimStart(void);
        double SimEnd(void);
        double SimStep(void);
        analysis_t AnalysisType(void);
        as_scale_t AnalysisScale(void);
        ODE_meth_t ODEMethod(void);
        return_codes_e errcode(void);
        bool valid(void);
        void clear(void);

    private:
        void init(void);
        return_codes_e setCircuitOptions(std::vector<std::string> &tokens);
        return_codes_e SPICECard(std::vector<std::string> &tokens, parser &match);
        return_codes_e verify(void);

        /* Debugging only functions */
        void debug_insert_nodes(node2_device &element);
        void debug_insert_nodes(node4_device &element);
        void debug_circuit(void);

        /* Elements contained in the circuit */
        std::vector<resistor> _res;     //!< Vector with all the resistors in the SPICE netlist.
        std::vector<capacitor> _caps;   //!< Vector with all the capacitors in the SPICE netlist.
        std::vector<coil> _coils;       //!< Vector with all the coils in the SPICE netlist.
        std::vector<ics> _ics;          //!< Vector with all the ICS in the SPICE netlist.
        std::vector<ivs> _ivs;          //!< Vector with all the IVS in the SPICE netlist.
        std::vector<vcvs> _vcvs;        //!< Vector with all the VCVS in the SPICE netlist.
        std::vector<vccs> _vccs;        //!< Vector with all the VCCS in the SPICE netlist.
        std::vector<ccvs> _ccvs;        //!< Vector with all the CCVS in the SPICE netlist.
        std::vector<cccs> _cccs;        //!< Vector with all the CCCS in the SPICE netlist.

        /* Elements/Nodes maps */
        hashmap_str_t _element_names;   //!< Hashtable that contains all the element names in the SPICE netlist.
        hashmap_str_t _nodes;           //!< Hashtable that contains all the nodes' names in the SPICE netlist.

        /* SPICE CARDS/OPTIONS - Analysis */
        double _sim_start;				//!< The simulation start value.
        double _sim_end;				//!< The simulation end value.
        double _sim_step;				//!< The simulation step value.
        as_scale_t _scale;				//!< Scale of the analysis.
        analysis_t _type;				//!< Analysis type.
        ODE_meth_t _ode_method;         //!< ODE method in case of transient.
        std::string _source;			//!< In case of DC analysis - Name of source.
        return_codes_e _errcode;        //!< Flag containing the last errorcode regarding the circuit.

        /* SPICE CARDS - Plot */
        std::vector<std::string> _plot_nodes;       //!< The nodes names to be plotted after simulation.
        std::vector<std::string> _plot_sources;     //!< The sources names to be plotted after simulation.
};

#endif // __CIRCUIT_H //
