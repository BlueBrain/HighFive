#include <highfive/H5File.hpp>
#include <highfive/H5Group.hpp>
#include <highfive/H5Attribute.hpp>

#include <iostream>

#define BOOST_TEST_MAIN GetScalars
#include <boost/test/unit_test.hpp>

#define TEST_FILE "scalar_attrs.h5"


using namespace HighFive;

/// Checks reading scalars from properties
///
BOOST_AUTO_TEST_CASE( GetScalars ) {

    File h5file(TEST_FILE, File::ReadWrite);
    Group g = h5file.getGroup("metadata");


    // std::string
    if( ! g.hasAttribute("family") ) {
        throw std::runtime_error("Attribute family not found");
    }

    Attribute att = g.getAttribute("family");
    std::string out;
    att.read(out);
    BOOST_CHECK_EQUAL( out, "NEURON" );


    // std::string
    if( ! g.hasAttribute("vlstr") ) {
        throw std::runtime_error("Attribute vlstr not found");
    }

    Attribute attx = g.getAttribute("vlstr");
    std::string outx;
    attx.read(outx);
    BOOST_CHECK_EQUAL( outx, "this is a VL string" );


    // int
    if( ! g.hasAttribute("one") ) {
        throw std::runtime_error("Attribute One not found");
    }

    Attribute att2 = g.getAttribute("one");
    int one;
    att2.read_scalar(one);
    BOOST_CHECK_EQUAL( one, 1 );


    // double
    if( ! g.hasAttribute("twodotone") ) {
        throw std::runtime_error("Attribute twodotone not found");
    }

    Attribute att3 = g.getAttribute("twodotone");
    double twodotone;
    att3.read_scalar(twodotone);
    BOOST_CHECK_CLOSE( twodotone, 2.1, 1e-8 );

}
