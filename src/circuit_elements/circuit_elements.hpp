#ifndef __CIRCUIT_ELEMENTS_HPP
#define __CIRCUIT_ELEMENTS_HPP

/* The base compositions */
#include "node2dev.hpp"
#include "source_spec.hpp"

/* All the available circuit elements */

/* Extended version - Used in during parsing and circuit formation */
class Coil : public node2_device{};
class Capacitor : public node2_device{};
class Resistor : public node2_device{};
class ivs : public node2_device, public source_spec{};
class ics : public node2_device, public source_spec{};

/* Packed versions - Used for simulator engine */
class coil_packed : public node2_device_packed{};
class capacitor_packed : public node2_device_packed{};
class resistor_packed : public node2_device_packed{};
class ivs_packed : public node2_device_packed, public source_spec{};
class ics_packed : public node2_device_packed, public source_spec{};

#endif // __CIRCUIT_ELEMENTS_HPP //
