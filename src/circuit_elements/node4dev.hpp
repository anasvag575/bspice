#ifndef __NODE4_DEV_HPP
#define __NODE4_DEV_HPP

#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

/* Complete representation */
class node4_device
{
    public:
        /*!
            @brief    Default constructor.
        */
        node4_device(void)
        {
            _value = 0;
            _nodes_ids[0] = -1;
            _nodes_ids[1] = -1;
            _nodes_ids[2] = -1;
            _nodes_ids[3] = -1;
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
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        IntTp getNegNodeID(void)
        {
            return this->_nodes_ids[1];
        }

        /*!
            @brief    Get the depended positive idx node of the device.
            @return   The idx.
        */
        IntTp getDepPosNodeID(void)
        {
            return this->_nodes_ids[2];
        }

        /*!
            @brief    Get the depended negative idx node of the device.
            @return   The idx.
        */
        IntTp getDepNegNodeID(void)
        {
            return this->_nodes_ids[3];
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
            @brief    Get the depended positive node name of the device.
            @return   The name.
        */
        const std::string &getDepPosNode(void)
        {
            return this->_node_names[2];
        }

        /*!
            @brief    Get the depended negative node name of the device.
            @return   The name.
        */
        const std::string &getDepNegNode(void)
        {
            return this->_node_names[3];
        }

        /*!
            @brief   Set the nodes of the device.
            @param   pos        The positive node name.
            @param   pos        The positive node name.
            @param   dep_pos    The depended positive node name.
            @param   dep_neg    The depended positive node name.
        */
        void setNodeNames(std::string &pos, std::string &neg, std::string &pos_dep, std::string &neg_dep)
        {
            this->_node_names[0] = pos;
            this->_node_names[1] = neg;
            this->_node_names[2] = pos_dep;
            this->_node_names[3] = neg_dep;
        }

        /*!
            @brief   Set the node IDs of the device.
            @param   pos        The positive node ID.
            @param   pos        The positive node ID.
            @param   dep_pos    The depended positive node ID.
            @param   dep_neg    The depended positive node ID.
        */
        void setNodeIDs(IntTp pos, IntTp neg, IntTp dep_pos, IntTp dep_neg)
        {
            this->_nodes_ids[0] = pos;
            this->_nodes_ids[1] = neg;
            this->_nodes_ids[2] = dep_pos;
            this->_nodes_ids[3] = dep_neg;
        }

        /*!
            @brief    Overloaded operator for stdout, that prints the contents of the
            element.
            @return   The reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, const node4_device& a)
        {
            os << a._name << " " ;
            os << a._nodes_ids[0] << " " << a._nodes_ids[1] << " " << a._nodes_ids[2] << " " << a._nodes_ids[3];
//            os << a._node_names[0] << " " << a._node_names[1] << " " << a._node_names[2] << " " << a._node_names[3];
            os << a._value << "\n";

            return os;
        }

        /*!
            @brief    Overloaded comparison operator, used when sorting
            elements in a container.
            @return   Boolean with the results of the comparison.
        */
        bool operator<(node4_device &b)
        {
            return this->_name < b._name;
        }

    private:
        std::string _name;                                     /* Name of element */
        double _value;                                         /* Amplification value of element */
        std::array<std::string, 4> _node_names;                /* The node names */
        std::array<IntTp, 4> _nodes_ids;                       /* The node IDs */
};

/* MNA packed structure */
class node4_device_packed
{
    public:
        /*!
            @brief    Default constructor.
        */
        node4_device_packed()
        {
            this->_nodes_ids[0] = -1;
            this->_nodes_ids[1] = -1;
            this->_nodes_ids[2] = -1;
            this->_nodes_ids[3] = -1;
            this->_value = 0;
        }

        /*!
            @brief      Constructor, converts extended to packed.
            @param  src The extended version of the device.
        */
        node4_device_packed(node4_device &src)
        {
            this->_nodes_ids[0] = src.getPosNodeID();
            this->_nodes_ids[1] = src.getNegNodeID();
            this->_nodes_ids[2] = src.getDepPosNodeID();
            this->_nodes_ids[3] = src.getDepNegNodeID();
            this->_value = src.getVal();
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
            @brief    Get the positive idx node of the device.
            @return   The idx.
        */
        IntTp getPosNodeID(void)
        {
            return this->_nodes_ids[0];
        }

        /*!
            @brief    Get the negative idx node of the device.
            @return   The idx.
        */
        IntTp getNegNodeID(void)
        {
            return this->_nodes_ids[1];
        }

        /*!
            @brief    Get the depended positive idx node of the device.
            @return   The idx.
        */
        IntTp getDepPosNodeID(void)
        {
            return this->_nodes_ids[2];
        }

        /*!
            @brief    Get the depended negative idx node of the device.
            @return   The idx.
        */
        IntTp getDepNegNodeID(void)
        {
            return this->_nodes_ids[3];
        }

    private:
        double _value;                                         /* Value in Element's SI units */
        std::array<IntTp, 4> _nodes_ids;                       /* The node IDs */
};

#endif // __NODE4_DEV_HPP //
