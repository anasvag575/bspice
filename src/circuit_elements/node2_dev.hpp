#ifndef __NODE2_DEV_HPP
#define __NODE2_DEV_HPP

#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

/**
 *  The complete device representation of node_2 devices (elements with 2 nodes associated with them).
 */
class node2_device
{
    public:
        /*!
            @brief    Default constructor.
        */
		node2_device(void)
		{
			_value = 0;
			_nodes_ids[0] = -1;
            _nodes_ids[1] = -1;
		}

        /*!
            @brief    Get the device's name
            @return   The name.
        */
		const std::string &Name(void) { return _name; }

        /*!
            @brief    Get the device's value
            @return   The value.
        */
        const double Val(void) { return _value; }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        const IntTp PosNodeID(void) { return _nodes_ids[0]; }

        /*!
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        const IntTp NegNodeID(void) { return _nodes_ids[1]; }

        /*!
            @brief    Get the positive node name of the device.
            @return   The name.
        */
        const std::string &PosNode(void) { return _node_names[0]; }

        /*!
            @brief    Get the negative node name of the device.
            @return   The name.
        */
        const std::string &NegNode(void) { return _node_names[1]; }

        /*!
            @brief    Set the device's name
            @param    The name.
        */
        void setName(const std::string &name) { _name = name; }

        /*!
            @brief    Set the value of the device
            @return   The value.
        */
        void setVal(const double val) { _value = val; }

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
            @param   pos    The positive node ID.
        */
        void setNodeIDs(const IntTp pos, const IntTp neg)
        {
            _nodes_ids[0] = pos;
            _nodes_ids[1] = neg;
        }

        /*!
            @brief    Overloaded operator for stdout, that prints the contents of the
            element.
            @return   The reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, const node2_device& a)
        {
        	os << a._name << " " ;
        	os << a._nodes_ids[0] << " " << a._nodes_ids[1] << " ";
//        	os << a._node_names[0] << " " << a._node_names[1] << " ";
        	os << a._value << "\n";

        	return os;
        }

        /*!
            @brief    Overloaded comparison operator, used when sorting
            elements in a container.
            @return   Boolean with the results of the comparison.
        */
        bool operator<(node2_device &b) { return this->_name < b._name; }

    private:
        std::string _name;									   //!< Name of element
        double _value;                                 		   //!< Value in Element's SI units
        std::array<std::string, 2> _node_names;  			   //!< The node names
        std::array<IntTp, 2> _nodes_ids;  					   //!< The node IDs
};

/**
 *  The packed device representation of node_2 devices (elements with 2 nodes associated with them).
 *  Used during MNA construction.
 */
class node2_device_packed
{
    public:
        /*!
            @brief    Default constructor.
        */
        node2_device_packed()
        {
            _nodes_ids[0] = -1;
            _nodes_ids[1] = -1;
            _value = 0;
        }

        /*!
            @brief      Constructor, converts extended to packed.
            @param  src The extended version of the device.
        */
        node2_device_packed(node2_device &src)
        {
            _nodes_ids[0] = src.PosNodeID();
            _nodes_ids[1] = src.NegNodeID();
            _value = src.Val();
        }

        /*!
            @brief    Get the device's value
            @return   The value.
        */
        double Val(void) { return _value; }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp PosNodeID(void) { return _nodes_ids[0]; }

        /*!
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        IntTp NegNodeID(void) { return _nodes_ids[1]; }

        /*!
            @brief    Set the value of the device
            @return   The value.
        */
        void setVal(double val) { _value = val; }

        /*!
            @brief   Set the node IDs of the device.
            @param   pos    The positive node ID.
            @param   pos    The positive node ID.
        */
        void setNodeIDs(IntTp pos, IntTp neg)
        {
            _nodes_ids[0] = pos;
            _nodes_ids[1] = neg;
        }

    private:
        double _value;                                         //!< Value in Element's (SI) units
        std::array<IntTp, 2> _nodes_ids;                       //!< The node IDs
};

#endif // __NODE2_DEV_HPP //
