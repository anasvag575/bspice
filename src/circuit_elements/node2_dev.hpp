#ifndef __NODE2_DEV_HPP
#define __NODE2_DEV_HPP

#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

//! The complete device representation of 2-node-basic devices (elements with 2 nodes associated with them).
/*!
  This class sets the representation used during SPICE netlist parsing (names and tokens are also kept),
  which are not needed for the simulation but serve as information for the user (in case of error).
*/
class node2_device
{
    public:
        /*!
            @brief    Default constructor.
        */
		node2_device() noexcept
		{
			_value = 0;
			_nodes_ids[0] = -1;
            _nodes_ids[1] = -1;
            _linenum = -1;
		}

        /*!
            @brief    Get the device's name
            @return   The name.
        */
		const std::string &Name(void) noexcept { return _name; }

        /*!
            @brief    Get the device's value
            @return   The value.
        */
        double Val(void) noexcept { return _value; }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp PosNodeID(void) noexcept { return _nodes_ids[0]; }

        /*!
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        IntTp NegNodeID(void) noexcept { return _nodes_ids[1]; }

        /*!
            @brief    Get the positive node name of the device.
            @return   The name.
        */
        const std::string &PosNode(void) noexcept { return _node_names[0]; }

        /*!
            @brief    Get the negative node name of the device.
            @return   The name.
        */
        const std::string &NegNode(void) noexcept { return _node_names[1]; }

        /*!
            @brief    Get the line number the element was found at in the spicenetlist.
            @return   The number.
        */
        const IntTp Linenum(void) noexcept { return _linenum; }

        /*!
            @brief    Set the device's name
            @param    name The name.
        */
        void setName(const std::string &name) { _name = name; }

        /*!
            @brief    Set the value of the device.
            @param    val The value.
        */
        void setVal(const double val) noexcept { _value = val; }

        /*!
            @brief   Set the nodes of the device.
            @param   pos    The positive node name.
            @param   neg    The negative node name.
        */
        void setNodeNames(const std::string &pos, const std::string &neg)
        {
            _node_names[0] = pos;
            _node_names[1] = neg;
        }

        /*!
            @brief   Set the node IDs of the device.
            @param   pos    The positive node ID.
            @param   neg    The negative node ID.
        */
        void setNodeIDs(const IntTp pos, const IntTp neg) noexcept
        {
            _nodes_ids[0] = pos;
            _nodes_ids[1] = neg;
        }

        /*!
            @brief    Set the number the device was found at in the netlist.
            @param    val The value.
        */
        void setLinenum(const IntTp val) noexcept { _linenum = val; }

        /*!
            @brief    Overloaded operator for stdout, that prints the contents of the
            element.
            @param    os    The stream reference.
            @param    dev   The device reference.
            @return   The   reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, const node2_device& dev)
        {
        	os << dev._linenum << ":"<<  dev._name << " " ;
        	os << dev._nodes_ids[0] << " " << dev._nodes_ids[1] << " ";
//        	os << dev._node_names[0] << " " << dev._node_names[1] << " ";
        	os << dev._value << "\n";

        	return os;
        }

        /*!
            @brief    Overloaded comparison operator, used when sorting
            elements in a container.
            @param    dev     The device reference
            @return         Boolean with the results of the comparison.
        */
        bool operator<(node2_device &dev) { return this->_name < dev._name; }

    private:
        std::string _name;									   //!< Name of element.
        double _value;                                 		   //!< Value in Element's SI units.
        std::array<std::string, 2> _node_names;  			   //!< The node names.
        std::array<IntTp, 2> _nodes_ids;  					   //!< The node IDs.
        IntTp _linenum;                                        //!< The line number the element was found at in the netlist.
};

//! The packed device (MNA) representation of 2-node-basic devices (elements with 2 nodes associated with them).
/*!
  This class sets the representation used during SPICE MNA formation and simulation (only IDs and values kept),
  increasing temporal locality during MNA reconstructions and reducing memory usage.
*/
class node2_device_packed
{
    public:
        /*!
            @brief    Default constructor.
        */
        node2_device_packed() noexcept
        {
            _nodes_ids[0] = -1;
            _nodes_ids[1] = -1;
            _value = 0;
        }

        /*!
            @brief      Constructor, converts extended to packed.
            @param  src The extended version of the device.
        */
        node2_device_packed(node2_device &src) noexcept
        {
            _nodes_ids[0] = src.PosNodeID();
            _nodes_ids[1] = src.NegNodeID();
            _value = src.Val();
        }

        /*!
            @brief    Get the device's value
            @return   The value.
        */
        double Val(void) noexcept { return _value; }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp PosNodeID(void) noexcept { return _nodes_ids[0]; }

        /*!
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        IntTp NegNodeID(void) noexcept { return _nodes_ids[1]; }

        /*!
            @brief    Set the value of the device
            @param    val The value.
        */
        void setVal(double val) noexcept { _value = val; }

        /*!
            @brief   Set the node IDs of the device.
            @param   pos    The positive node ID.
            @param   neg    The positive node ID.
        */
        void setNodeIDs(IntTp pos, IntTp neg) noexcept
        {
            _nodes_ids[0] = pos;
            _nodes_ids[1] = neg;
        }

    private:
        double _value;                                         //!< Value in Element's (SI) units.
        std::array<IntTp, 2> _nodes_ids;                       //!< The node IDs.
};

#endif // __NODE2_DEV_HPP //
