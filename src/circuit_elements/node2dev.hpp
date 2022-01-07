#ifndef __NODE2_DEV_HPP
#define __NODE2_DEV_HPP

#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

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
        std::string &getName(void)
        {
            return this->_name;
        }

        /*!
            @brief    Set the device's name
            @param    The name.
        */
        void setName(std::string &name)
        {
            this->_name = name;
        }

        /*!
            @brief    Get the device's value
            @return   The value.
        */
        double getVal(void)
        {
            return _value;
        }

        /*!
            @brief    Set the value of the device
            @return   The value.
        */
        void setVal(double val)
        {
        	this->_value = val;
        }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp getPosNodeID(void)
        {
            return this->_nodes_ids[0];
        }

        /*!
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp getNegNodeID(void)
        {
            return this->_nodes_ids[1];
        }

        /*!
            @brief    Get the positive node name of the device.
            @return   The idx.
        */
        const std::string &getPosNode(void)
        {
            return this->_node_names[0];
        }

        /*!
            @brief    Get the positive node name of the device.
            @return   The idx.
        */
        const std::string &getNegNode(void)
        {
            return this->_node_names[1];
        }

        /*!
            @brief   Set the nodes of the device.
            @param   pos    The positive node name.
            @param   neg    The negative node name.
        */
        void setNodeNames(std::string &pos, std::string &neg)
        {
            this->_node_names[0] = pos;
            this->_node_names[1] = neg;
        }

        /*!
            @brief   Set the node IDs of the device.
            @param   pos    The positive node ID.
            @param   pos    The positive node ID.
        */
        void setNodeIDs(IntTp pos, IntTp neg)
        {
            this->_nodes_ids[0] = pos;
            this->_nodes_ids[1] = neg;
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
        bool operator<(node2_device &b)
        {
            return this->_name < b._name;
        }

    private:
        std::string _name;									   /* Name of element */
        double _value;                                 		   /* Value in Element's SI units */
        std::array<std::string, 2> _node_names;  			   /* The node names */
        std::array<IntTp, 2> _nodes_ids;  					   /* The node IDs */
};

#endif // __NODE2_DEV_HPP //
