#ifndef __CIRCUIT_ELEMENTS_HPP
#define __CIRCUIT_ELEMENTS_HPP

/* The base compositions */
#include "node2_dev.hpp"
#include "node2s_dev.hpp"
#include "node4_dev.hpp"
#include "source_spec.hpp"

//! The coil class, used during circuit formation.
class coil : public node2_device{};

//! The capacitor class, used during circuit formation.
class capacitor : public node2_device{};

//! The resistor class, used during circuit formation.
class resistor : public node2_device{};

//! The IVS (Independent Voltage Source) class, used during circuit formation.
class ivs : public node2_device, public source_spec{};

//! The ICS (Independent Current Source) class, used during circuit formation.
class ics : public node2_device, public source_spec{};

//! The VCVS (Voltage Controlled Voltage Source) class, used during circuit formation.
class vcvs : public node4_device{};

//! The VCCS (Voltage Controlled Current Source) class, used during circuit formation.
class vccs : public node4_device{};

//! The CCVS (Current Controlled Voltage Source) class, used during circuit formation.
class ccvs : public node2s_device{};

//! The CCCS (Current Controlled Current Source) class, used during circuit formation.
class cccs : public node2s_device{};




//! The packed coil class, used during MNA formation.
class coil_packed : public node2_device_packed{};

//! The packed capacitor class, used during MNA formation.
class capacitor_packed : public node2_device_packed{};

//! The packed resistor class, used during MNA formation.
class resistor_packed : public node2_device_packed{};

//! The packed IVS (Independent Voltage Source) class, used during MNA formation.
class ivs_packed : public node2_device_packed, public source_spec{};

//! The packed ICS (Independent Current Source) class, used during MNA formation.
class ics_packed : public node2_device_packed, public source_spec{};

//! The packed VCVS (Voltage Controlled Voltage Source) class, used during MNA formation.
class vcvs_packed : public node4_device_packed{};

//! The packed VCCS (Voltage Controlled Current Source) class, used during MNA formation.
class vccs_packed : public node4_device_packed{};

//! The packed CCVS (Current Controlled Voltage Source) class, used during MNA formation.
class ccvs_packed : public node2s_device_packed{};

//! The packed CCCS (Current Controlled Current Source) class, used during MNA formation.
class cccs_packed : public node2s_device_packed{};

#endif // __CIRCUIT_ELEMENTS_HPP //
