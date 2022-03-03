#ifndef __NODE2S_DEV_HPP
#define __NODE2S_DEV_HPP

#include "node2_dev.hpp"  /* Base class */
#include <string>
#include <array>
#include <iostream>
#include "base_types.hpp"

//! The complete device representation of 2-node-source (elements with 2 nodes associated with them and a depended source).
/*!
  This class sets the representation used during SPICE netlist parsing (names and tokens are also kept),
  which are not needed for the simulation but serve as information for the user (in case of error).
*/
class node2s_device : public node2_device
{
    public:
        /*!
            @brief    Default constructor.
        */
		node2s_device(void)
		{
		    _source_id = -1;
		}

        /*!
            @brief    Get the depended source name.
            @return   The name
        */
		const std::string SourceName(void) { return _source_name; }

        /*!
            @brief    Get the depended source ID.
            @return   The ID.
        */
        IntTp SourceID(void) { return _source_id; }

        /*!
            @brief    Set the depended source name.
            @param    source_name  The source name
        */
        void SetSourceName(const std::string source_name) { _source_name = source_name; }

        /*!
            @brief    Set the depended source ID.
            @param    source_id   The ID.
        */
        void SetSourceID(IntTp source_id) { _source_id = source_id; }

        /*!
            @brief    Overloaded operator for stdout, that prints the contents of the
            element.
            @param      os  The reference to the stream.
            @param      dev The reference to the device.
            @return         The reference to the stream.
        */
        friend std::ostream &operator<<(std::ostream& os, node2s_device& dev)
        {
        	os << dev.Name() << " " ;
            os << dev.PosNodeID() << " " << dev.NegNodeID() << " " << dev.SourceID() << " ";
//            os << dev.PosNode() << " " << dev.NegNode() << " " << dev.SourceName() << " ";;
        	os << dev.Val() << "\n";

        	return os;
        }

    private:
        std::string _source_name;   //!< The depended source name
        IntTp _source_id;           //!< The depended source ID
};

//! The packed device (MNA) representation of 2-node-source (elements with 2 nodes associated with them and a depended source).
/*!
  This class sets the representation used during SPICE MNA formation and simulation (only IDs and values kept),
  increasing temporal locality during MNA reconstructions and reducing memory usage.
*/
class node2s_device_packed : public node2_device_packed
{
    public:
        /*!
            @brief    Default constructor.
        */
        node2s_device_packed()
        {
            _source_id = -1;
        }

        /*!
            @brief      Constructor, converts extended to packed.
            @param  src The extended version of the device.
        */
        node2s_device_packed(node2s_device &src)
        {
            setNodeIDs(src.PosNodeID(), src.NegNodeID());
            setVal(src.Val());
            _source_id = src.SourceID();
        }

        /*!
            @brief    Get the depended source ID.
            @return   The ID.
        */
        IntTp SourceID(void) { return _source_id; }

    private:
        IntTp _source_id;    //!< The depended source ID
};

#endif // __NODE2_DEV_HPP //
