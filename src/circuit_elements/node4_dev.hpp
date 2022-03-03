#ifndef __NODE4_DEV_HPP
#define __NODE4_DEV_HPP

#include "node2_dev.hpp" /* Base class */
#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

//! The complete device representation of 4-node-basic (elements with 4 nodes associated with them).
/*!
  This class sets the representation used during SPICE netlist parsing (names and tokens are also kept),
  which are not needed for the simulation but serve as information for the user (in case of error).
*/
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
            @brief    Get the depended positive node ID of the device.
            @return   The idx.
        */
        IntTp DepPosNodeID(void) { return _dep_nodes_ids[0]; }

        /*!
            @brief    Get the depended negative node ID of the device.
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
            @param    os    The stream reference.
            @param    dev   The device reference.
            @return   The   reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, node4_device& dev)
        {
            os << dev.Name() << " " ;
            os << dev.PosNodeID() << " " << dev.NegNodeID() << " " << dev.DepPosNodeID() << " " << dev.DepNegNodeID() << " ";
//            os << dev.PosNode() << " " << dev.NegNode() << " " << dev.DepPosNode() << " " << dev.DepNegNode() << " ";
            os << dev.Val() << "\n";

            return os;
        }

    private:
        std::array<std::string, 2> _dep_node_names;            //!< The depended node names
        std::array<IntTp, 2> _dep_nodes_ids;                   //!< The depended node IDs
};

//! The packed device (MNA) representation of 4-node-basic devices (elements with 4 nodes associated with them).
/*!
  This class sets the representation used during SPICE MNA formation and simulation (only IDs and values kept),
  increasing temporal locality during MNA reconstructions and reducing memory usage.
*/
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
