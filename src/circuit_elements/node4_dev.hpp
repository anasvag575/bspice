#ifndef __NODE4_DEV_HPP
#define __NODE4_DEV_HPP

#include "node2_dev.hpp" /* Base class */
#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

//! The complete device representation of node_4 devices (elements with 4 nodes associated with them).
class node4_device : public node2_device
{
    public:
        /*!
            @brief    Default constructor.
        */
        node4_device(void)
        {
            _dep_nodes_ids[0] = -1;
            _dep_nodes_ids[1] = -1;
        }

        /*!
            @brief    Get the depended positive idx node of the device.
            @return   The idx.
        */
        IntTp DepPosNodeID(void) { return _dep_nodes_ids[0]; }

        /*!
            @brief    Get the depended negative idx node of the device.
            @return   The idx.
        */
        IntTp DepNegNodeID(void) { return _dep_nodes_ids[1]; }

        /*!
            @brief    Get the depended positive node name of the device.
            @return   The name.
        */
        const std::string &DepPosNode(void) { return _dep_node_names[0]; }

        /*!
            @brief    Get the depended negative node name of the device.
            @return   The name.
        */
        const std::string &DepNegNode(void) { return _dep_node_names[1]; }

        /*!
            @brief   Set the depended nodes names of the device.
            @param   pos_dep    The depended positive node name.
            @param   neg_dep    The depended negative node name.
        */
        void setDepNodeNames(const std::string &pos_dep, const std::string &neg_dep)
        {
            _dep_node_names[0] = pos_dep;
            _dep_node_names[1] = neg_dep;
        }

        /*!
            @brief   Set the depended node IDs of the device.
            @param   dep_pos    The depended positive node ID.
            @param   dep_neg    The depended negative node ID.
        */
        void setDepNodeIDs(const IntTp dep_pos, const IntTp dep_neg)
        {
            _dep_nodes_ids[0] = dep_pos;
            _dep_nodes_ids[1] = dep_neg;
        }

        /*!
            @brief    Overloaded operator for stdout, that prints the contents of the
            element.
            @return   The reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, node4_device& a)
        {
            os << a.Name() << " " ;
            os << a.PosNodeID() << " " << a.NegNodeID() << " " << a.DepPosNodeID() << " " << a.DepNegNodeID() << " ";
//            os << a.PosNode() << " " << a.NegNode() << " " << a.DepPosNode() << " " << a.DepNegNode() << " ";
            os << a.Val() << "\n";

            return os;
        }

    private:
        std::array<std::string, 2> _dep_node_names;            //!< The depended node names
        std::array<IntTp, 2> _dep_nodes_ids;                   //!< The depended node IDs
};

//! The packed device (MNA)representation of node_4 devices (elements with 4 nodes associated with them).
class node4_device_packed : public node2_device_packed
{
    public:
        /*!
            @brief    Default constructor.
        */
        node4_device_packed()
        {
            _dep_nodes_ids[0] = -1;
            _dep_nodes_ids[1] = -1;
        }

        /*!
            @brief      Constructor, converts extended to packed.
            @param  src The extended version of the device.
        */
        node4_device_packed(node4_device &src)
        {
            this->setNodeIDs(src.PosNodeID(), src.NegNodeID());
            this->setVal(src.Val());
            _dep_nodes_ids[0] = src.DepPosNodeID();
            _dep_nodes_ids[1] = src.DepNegNodeID();
        }

        /*!
            @brief    Get the depended positive idx node of the device.
            @return   The idx.
        */
        IntTp DepPosNodeID(void) { return _dep_nodes_ids[0]; }

        /*!
            @brief    Get the depended negative idx node of the device.
            @return   The idx.
        */
        IntTp DepNegNodeID(void) { return _dep_nodes_ids[1]; }

    private:
        std::array<IntTp, 2> _dep_nodes_ids;                   //!< The node IDs
};

#endif // __NODE4_DEV_HPP //
