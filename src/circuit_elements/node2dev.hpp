#ifndef __NODE2_DEV_HPP
#define __NODE2_DEV_HPP

#include <string>
#include <array>
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
            @brief    Get the nodes (names) of the device.
            @return   The array of nodes.
        */
        std::array<std::string, 2> &getNodeNames(void)
        {
        	return this->_node_names;
        }

        /*!
            @brief    Get the nodes (IDs) of the device.
            @return   The array of nodes.
        */
        std::array<IntTp, 2> &getNodeIDs(void)
        {
        	return this->_nodes_ids;
        }

    private:
        std::string _name;									   /* Name of element */
        double _value;                                 		   /* Value in Element's SI units */
        std::array<std::string, 2> _node_names;  			   /* The node names */
        std::array<IntTp, 2> _nodes_ids;  					   /* The node IDs */
};

#endif // __NODE2_DEV_HPP //
